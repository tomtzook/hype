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

namespace efi::result {

const common::result_category_t CATEGORY = 2;

common::result efi_result(EFI_STATUS efi_status) noexcept;

#ifdef _DEBUG
namespace debug {
const wchar_t* to_string(const common::result& result) noexcept;
}
#endif

}

template<>
common::result::result_t(EFI_STATUS code) noexcept;
