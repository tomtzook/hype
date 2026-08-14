#include <x86/cpuid.h>
#include <x86/io.h>

#include <efi/efi_base.h>
#include <base.h>
#include <efi/efi_environment.h>

#include "config.h"
#include "hype.h"
#include "debug/hype_gdbstub.h"


static void disable_qemu_timers() {
    // Offset for the LVT Timer Register is 0x320
    volatile uint32_t* lvt_timer = reinterpret_cast<uint32_t*>(0xFEE00000 + 0x320);
    // Set bit 16 to '1' to mask the interrupt
    *lvt_timer |= (1 << 16);
    // Port 0x21 is the Offset for the Master PIC Mask Register
    // Setting Bit 0 masks IRQ0 (the timer)
    x86::outb(0x21, x86::inb(0x21) | 0x01);
}

static framework::result<> init(EFI_HANDLE image_handle) {
    verify(environment::initialize_efi(image_handle));
    return {};
}

static framework::result<> start() {
    verify(hype::initialize());

    const auto start_result = hype::start();
    if (!start_result) {
        hype::free();

        return start_result;
    }

    return {};
}

static EFI_STATUS load_image(EFI_HANDLE ImageHandle, EFI_HANDLE FSHandle, CHAR16* TargetFilePath, EFI_HANDLE* LoadedImageHandleOut) {
    EFI_STATUS Status;
    EFI_DEVICE_PATH_PROTOCOL* DevicePath = NULL;
    Status = gBS->OpenProtocol(
        FSHandle,
        &gEfiDevicePathProtocolGuid,
        reinterpret_cast<void**>(&DevicePath),
        ImageHandle,
        nullptr,
        EFI_OPEN_PROTOCOL_GET_PROTOCOL);
    if (EFI_ERROR(Status)) {
        return Status;
    }

    EFI_DEVICE_PATH_PROTOCOL* FullFilePath = FileDevicePath(FSHandle, TargetFilePath);
    EFI_HANDLE LoadedImageHandle = NULL;
    Status = gBS->LoadImage(
        false,
        ImageHandle,
        FullFilePath,
        nullptr,
        0,
        &LoadedImageHandle);
    if (EFI_ERROR(Status)) {
        return Status;
    }

    *LoadedImageHandleOut = LoadedImageHandle;
    return EFI_SUCCESS;
}

static EFI_STATUS load_os(EFI_HANDLE ImageHandle, const wchar_t* TargetFilePathRaw) {
    EFI_STATUS Status;
    UINTN HandleCount = 0;
    EFI_HANDLE* HandleBuffer = nullptr;
    CHAR16* TargetFilePath = reinterpret_cast<CHAR16*>(const_cast<wchar_t*>(TargetFilePathRaw));

    Status = gBS->LocateHandleBuffer(
        ByProtocol,
        &gEfiSimpleFileSystemProtocolGuid,
        nullptr,
        &HandleCount,
        &HandleBuffer);
    if (EFI_ERROR(Status)) {
        trace_error("Failed to locate SimpleFileSystemProtocol Handles");
        return Status;
    }

    EFI_HANDLE TargetHandle = nullptr;
    for (int i = 0; i < HandleCount; ++i) {
        EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* FileSystem = nullptr;
        Status = gBS->OpenProtocol(
            HandleBuffer[i],
            &gEfiSimpleFileSystemProtocolGuid,
            reinterpret_cast<void**>(&FileSystem),
            ImageHandle,
            nullptr,
            EFI_OPEN_PROTOCOL_GET_PROTOCOL);
        if (EFI_ERROR(Status)) {
            continue;
        }

        EFI_FILE_PROTOCOL* Root = nullptr;
        Status = FileSystem->OpenVolume(FileSystem, &Root);
        if (EFI_ERROR(Status)) {
            continue;
        }

        EFI_FILE_PROTOCOL* File = nullptr;
        Status = Root->Open(Root, &File, TargetFilePath, EFI_FILE_MODE_READ, 0);

        if (File != nullptr) {
            File->Close(File);
        }
        Root->Close(Root);

        if (!EFI_ERROR(Status)) {
            TargetHandle = HandleBuffer[i];
            break;
        }
    }

    if (TargetHandle == nullptr) {
        // did not find
        FreePool(HandleBuffer);
        return EFI_NOT_FOUND;
    }

    EFI_HANDLE LoadedImageHandle = nullptr;
    Status = load_image(ImageHandle, TargetHandle, TargetFilePath, &LoadedImageHandle);
    if (EFI_ERROR(Status)) {
        return Status;
    }

    FreePool(HandleBuffer);

    Status = gBS->StartImage(LoadedImageHandle, nullptr, nullptr);
    if (EFI_ERROR(Status)) {
        return Status;
    }

    return EFI_SUCCESS;
}

extern "C"
EFI_STATUS EFIAPI
UefiMain(
        IN EFI_HANDLE ImageHandle,
        IN EFI_SYSTEM_TABLE* SystemTable
) {
    trace_debug("entry");

    disable_qemu_timers();

    {
        const auto result = init(ImageHandle);
        if (!result) {
            trace_status("initialization failed", result.error());
            return EFI_LOAD_ERROR;
        }
    }

    {
        const auto result = environment::get_our_image_info();
        if (!result) {
            trace_status("Failed to get our image information", result.error());
            return EFI_LOAD_ERROR;
        }

        const auto& info = result.value();
        trace_debug("Main Start: base=0x%llx, end=0x%llx, size=0x%llx", info.base, reinterpret_cast<uint64_t>(info.base) + info.size, info.size);
    }

    if constexpr (hype::config::embedded_gdbstub) {
        gdbstub::initialize();

        if (hype::config::wait_for_gdb_connect) {
            gdbstub::wait_for_server();
        }
    }

    {
        const auto result = start();
        if (result) {
            trace_debug("Hypervisor Launched");
        } else {
            trace_status("start failed", result.error());
            catastrophic_error("failed to start hypervisor")
        }
    }

    trace_debug("launching OS");
    load_os(ImageHandle, L"EFI\\Microsoft\\Boot\\bootmgfw.efi");

    return EFI_SUCCESS;
}

