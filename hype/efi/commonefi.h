#pragma once

extern "C" {

#include <Base.h>
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/PrintLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

}

#include <common/common.h>

namespace hype::result {

status efi_status(EFI_STATUS code) noexcept;

}
