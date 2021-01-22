
#include "common.h"
#include "commonefi.h"
#include "hype.h"


extern "C"
EFI_STATUS EFIAPI
efi_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE* system_table) {
    common::result status = hype::result::SUCCESS;

    InitializeLib(image_handle, system_table);
    TRACE_DEBUG("Hello from the UEFI");

    CHECK(hype::initialize());
    CHECK(hype::start());

    TRACE_DEBUG("THIS IS THE END");
cleanup:
    hype::free();

    TRACE_DEBUG("Finished with status: %d", status.code());
    DEBUG_ONLY(common::debug::deadloop());

    return EFI_SUCCESS;
}
