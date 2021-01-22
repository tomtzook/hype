#pragma once

#include <types.h>
#include <error.h>


namespace x86::vmx {

common::result _vmxon(physical_address_t address) noexcept;
common::result _vmxoff() noexcept;

common::result _vmclear(physical_address_t address) noexcept;

}
