#pragma once

#include <error.h>

#include "x86/vmx/environment.h"


namespace x86::vmx {

common::result on(vmxon_region_t& vmxon_region) noexcept;
common::result off() noexcept;

common::result read(vmcs_t& vmcs) noexcept;
common::result write(vmcs_t& vmcs) noexcept;

}
