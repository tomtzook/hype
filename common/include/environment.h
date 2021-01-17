#pragma once

#include <types.h>

#include "error.h"

namespace environment {

enum class alignment_t {
    NO_ALIGN,
    PAGE_ALIGN
};

common::result allocate(size_t size, void** out, alignment_t alignment = alignment_t::NO_ALIGN) noexcept;

template<typename alloc_type>
common::result allocate(size_t size, alloc_type** out, alignment_t alignment = alignment_t::NO_ALIGN) noexcept{
    return allocate(size, reinterpret_cast<void**>(out), alignment);
}

common::result free(void* memory) noexcept;

uintn_t to_physical(void* address) noexcept;

}
