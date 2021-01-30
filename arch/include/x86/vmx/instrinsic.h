#pragma once

#include <types.h>
#include <error.h>


namespace x86::vmx {

common::result _vmxon(physical_address_t address) noexcept;
common::result _vmxoff() noexcept;

common::result _vmclear(physical_address_t address) noexcept;
common::result _vmread(uint32_t field, uintn_t& value) noexcept;
common::result _vmwrite(uint32_t field, uintn_t value) noexcept;
common::result _vmptrld(physical_address_t address) noexcept;
common::result _vmptrst(void* address) noexcept;

common::result _vmlaunch() noexcept;

}
