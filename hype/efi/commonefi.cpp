

#include "commonefi.h"


namespace hype::result {

status efi_status(EFI_STATUS code) noexcept {
    return {category_t::EFI, code};
}

}
