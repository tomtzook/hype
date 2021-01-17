#pragma once

#include <types.h>
#include <error.h>


namespace x86::vmx {

common::result vmxon(uintn_t address) noexcept;
common::result vmxoff() noexcept;

}
