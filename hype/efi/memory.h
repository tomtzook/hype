#pragma once

#include <common/common.h>
#include <environment/allocation.h>


namespace hype::efi::memory {

result::status allocate(void*& out, size_t size, hype::memory::type_t type) noexcept;
void free(void* ptr) noexcept;

}
