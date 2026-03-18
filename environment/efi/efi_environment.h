#pragma once

#include "efi_base.h"
#include "environment.h"

namespace environment {

framework::result<> initialize_efi(EFI_HANDLE image_handle);

}
