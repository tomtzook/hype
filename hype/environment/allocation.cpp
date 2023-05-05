
#include <x86/paging/paging.h>

#include <efi/memory.h>
#include "allocation.h"


namespace hype::environment {

static constexpr size_t ALLOCATION_HEADER_SIZE = sizeof(uintptr_t);

static size_t alignment_to_size(memory::alignment_t alignment) {
    switch (alignment) {
        case memory::alignment_t::PAGE_ALIGN: return x86::paging::page_size_4k;
        default: return 0;
    }
}

result::status allocate(size_t size,
                        void*& out,
                        memory::alignment_t alignment,
                        memory::type_t type) noexcept {
    void* memory;
    void* aligned_mem;
    uintptr_t* header;
    size_t alignment_size = alignment_to_size(alignment);

    CHECK_AND_RETURN(efi::memory::allocate(memory, size + ALLOCATION_HEADER_SIZE + alignment_size - 1, type));
    
    if (memory::alignment_t::NO_ALIGN != alignment) {
        aligned_mem = reinterpret_cast<void*>((reinterpret_cast<uintptr_t>(memory) + ALLOCATION_HEADER_SIZE  + alignment_size - 1)
                      & ~(uintptr_t)(alignment_size - 1));
    } else {
        aligned_mem = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(memory) + ALLOCATION_HEADER_SIZE);
    }

    header = reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(aligned_mem) - ALLOCATION_HEADER_SIZE);
    *header = reinterpret_cast<uintptr_t>(memory);

    out = aligned_mem;

    return {};
}

void free(void* ptr) noexcept {
    if (nullptr == ptr) {
        return;
    }

    auto header = reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(ptr) - ALLOCATION_HEADER_SIZE);
    ptr = reinterpret_cast<void*>(*header);

    efi::memory::free(ptr);
}

}
