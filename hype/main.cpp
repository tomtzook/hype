#include "efi/efi_base.h"

#include "base.h"
#include "cpu.h"
#include "hype.h"


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

    const auto result = start();
    if (result) {
        trace_debug("Hypervisor Launched");
        trace_debug("yooooo");
    } else {
        trace_status(result.error(), "start failed");
    }

    hype::hlt_cpu();
    //framework::debug::deadloop();

    return EFI_SUCCESS;
}

