#pragma once

#include <status.h>

extern "C" {

#include <Base.h>
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/PrintLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Protocol/LoadedImage.h>
#include <Pi/PiMultiPhase.h>
#include <Protocol/MpService.h>

}

namespace efi {

static constexpr framework::status_category_t status_category_efi = 2;

}

#define verify_efi(...)              \
    do {                                \
        auto __status = __VA_ARGS__;    \
        if (__status != EFI_SUCCESS) {                \
            return framework::err(framework::status(efi::status_category_efi, __status));\
        }                               \
    } while (false);
