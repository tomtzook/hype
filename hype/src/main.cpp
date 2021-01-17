
#include "common.h"
#include "commonefi.h"
#include "hype.h"


extern "C"
EFI_STATUS EFIAPI
efi_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE* system_table) {
    common::result status = hype::result::SUCCESS;

    InitializeLib(image_handle, system_table);
    TRACE_DEBUG("Hello from the UEFI");

    hype::context_t* context = nullptr;
    CHECK(hype::initialize(context));

    // do something with the context

    TRACE_DEBUG("THIS IS THE END");
cleanup:
    if (nullptr != context) {
        common::result free_status = hype::free(context);
        if (!free_status) {
            TRACE_ERROR("Error freeing context %d", free_status.code());
        }
    }

    TRACE_DEBUG("Finished with status: %d", status.code());
    common::debug::deadloop();

    return EFI_SUCCESS;
}
