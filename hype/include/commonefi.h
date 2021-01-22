#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <efi.h>
#include <efilib.h>

#ifdef __cplusplus
}
#endif

#include "common.h"

namespace efi {

class efi_service_t;

common::result initialize(efi_service_t& service) noexcept;

namespace result {

const common::result_category_t CATEGORY = 2;

common::result efi_result(EFI_STATUS efi_status) noexcept;

#ifdef _DEBUG
namespace debug {
const wchar_t* to_string(const common::result& result) noexcept;
} // namespace debug
#endif

} // namespace result


class efi_service_t {
public:
    bool is_after_exit_boot_services() noexcept;

    common::result allocate(size_t size, void** out) noexcept;
    common::result free(void* memory) noexcept;

    friend common::result efi::initialize(efi_service_t& service) noexcept;
};

}

template<>
common::result::result_t(EFI_STATUS code) noexcept;
