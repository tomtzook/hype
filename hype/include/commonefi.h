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

namespace hype::efi {

common::result efi_result(EFI_STATUS efi_status) noexcept;

}
