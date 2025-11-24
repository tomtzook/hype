#include "efi/efi_base.h"

#include <base.h>
#include "cpu.h"
#include "environment.h"
#include "hype.h"


static framework::result<> init() {
    verify(environment::initialize());
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

extern "C"
EFI_STATUS EFIAPI
UefiMain(
        IN EFI_HANDLE ImageHandle,
        IN EFI_SYSTEM_TABLE* SystemTable
) {
    EFI_LOADED_IMAGE_PROTOCOL* loaded_image{};
    const auto _efiStatus = SystemTable->BootServices->HandleProtocol(ImageHandle, &gEfiLoadedImageProtocolGuid, reinterpret_cast<void**>(&loaded_image));
    if (_efiStatus != EFI_SUCCESS) {
        trace_error("Failed locating loaded image: 0x%lx", _efiStatus);
        return _efiStatus;
    }

    trace_debug("Main Start: base=0x%llx, size=0x%llx", loaded_image->ImageBase, loaded_image->ImageSize);

    {
        const auto result = init();
        if (!result) {
            trace_status("initialization failed", result.error());
            return EFI_LOAD_ERROR;
        }
    }

    {
        const auto result = start();
        if (result) {
            trace_debug("Hypervisor Launched");
            __asm__ volatile ("cli; hlt");
        } else {
            trace_status("start failed", result.error());
            __asm__ volatile ("cli; hlt");
        }
    }

    return EFI_SUCCESS;
}

