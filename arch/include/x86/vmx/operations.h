#pragma once

#include <error.h>


namespace x86::vmx {

common::result on(void*& vmxon_region) noexcept;

common::result off() noexcept;

}
