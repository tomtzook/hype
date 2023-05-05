#include <efi/commonefi.h>

#include "hype.h"


extern "C"
EFI_STATUS EFIAPI
UefiMain(
        IN EFI_HANDLE ImageHandle,
        IN EFI_SYSTEM_TABLE* SystemTable
) {
    hype::result::status status = {};

    TRACE_DEBUG("Main Start");

    CHECK_AND_JUMP(status, hype::initialize());
    CHECK_AND_JUMP(status, hype::start());

cleanup:
    hype::free();

    TRACE_DEBUG("Finished");
    TRACE_STATUS(status);

    DEBUG_ONLY(hype::debug::deadloop());

    return EFI_SUCCESS;
}

