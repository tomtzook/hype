#include "efi_base.h"

#include "base.h"
#include "hype.h"

extern "C"
EFI_STATUS EFIAPI
UefiMain(
        IN EFI_HANDLE ImageHandle,
        IN EFI_SYSTEM_TABLE* SystemTable
) {
    hype::status_t status{};

    EFI_LOADED_IMAGE_PROTOCOL* loaded_image{};
    auto _efiStatus = SystemTable->BootServices->HandleProtocol(ImageHandle, &gEfiLoadedImageProtocolGuid, reinterpret_cast<void**>(&loaded_image));
    if (_efiStatus != EFI_SUCCESS) {
        TRACE_ERROR("Failed locating loaded image: 0x%lx", _efiStatus);
        return _efiStatus;
    }

    TRACE_DEBUG("Main Start: base=0x%llx, size=0x%llx", loaded_image->ImageBase, loaded_image->ImageSize);

    CHECK_AND_JUMP(cleanup, status, hype::initialize());
    CHECK_AND_JUMP(cleanup, status, hype::start());

cleanup:
    hype::free();

    TRACE_DEBUG("Finished");
    TRACE_STATUS(status);

    DEBUG_ONLY(hype::debug::deadloop());

    return EFI_SUCCESS;
}

