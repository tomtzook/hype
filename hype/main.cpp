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

    TRACE_DEBUG("Main Start");

    CHECK_AND_JUMP(cleanup, status, hype::initialize());
    CHECK_AND_JUMP(cleanup, status, hype::start());

cleanup:
    hype::free();

    TRACE_DEBUG("Finished");
    TRACE_STATUS(status);

    DEBUG_ONLY(hype::debug::deadloop());

    return EFI_SUCCESS;
}

