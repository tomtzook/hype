#pragma once

#include <x86/memory.h>
#include <types.h>

#include "error.h"

namespace hype::environment {

enum class alignment_t : size_t {
    NO_ALIGN = 0,
    PAGE_ALIGN = x86::PAGE_SIZE
};

hype::result allocate(size_t size, void** out, alignment_t alignment = alignment_t::NO_ALIGN) noexcept;

template<typename alloc_type>
hype::result allocate(size_t size, alloc_type** out, alignment_t alignment = alignment_t::NO_ALIGN) noexcept{
    return allocate(size, reinterpret_cast<void**>(out), alignment);
}

hype::result free(void* memory) noexcept;

}
