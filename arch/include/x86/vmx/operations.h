#pragma once

#include <error.h>


namespace x86::vmx {

common::result initialize_vmxon_region(vmxon_region_t& vmxon_region) noexcept;
common::result on(physical_address_t vmxon_region) noexcept;

common::result off() noexcept;

}
