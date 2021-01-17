
#include "common.h"

#include "commonefi.h"


common::result hype::efi::efi_result(EFI_STATUS efi_status) noexcept {
    return common::result(efi_status, common::result_category_t::EFI);
}
