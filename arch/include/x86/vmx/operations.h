#pragma once

#include <error.h>


namespace x86::vmx {

common::result on(vmxon_region_t& vmxon_region) noexcept;
common::result off() noexcept;

common::result clear(vmcs_t& vmcs) noexcept;

}
