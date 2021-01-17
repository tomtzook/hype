
#include "commonefi.h"


hype::result hype::efi::efi_result(EFI_STATUS efi_status) noexcept {
    return result(efi_status, hype::result_category_t::EFI);
}
