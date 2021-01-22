#include <x86/memory.h>

#include "commonefi.h"
#include "common.h"
#include "hype.h"

#include <environment.h>


static constexpr size_t ALLOCATION_HEADER_SIZE = sizeof(uintptr_t);


static size_t alignment_to_size(environment::alignment_t alignment) {
    switch (alignment) {
        case environment::alignment_t::PAGE_ALIGN: return x86::PAGE_SIZE;
        default: return 0;
    }
}


common::result environment::allocate(size_t size,
                                               void** out,
                                               alignment_t alignment) noexcept {
    common::result status = common::result::SUCCESS;

    void* memory;
    void* aligned_mem;
    uintptr_t* header;
    size_t alignment_size = alignment_to_size(alignment);

    CHECK(hype::g_context->environment.get_efi_service().allocate(
            size + alignment_size - 1,
            &memory));

    if (alignment_t::NO_ALIGN != alignment) {
        aligned_mem = reinterpret_cast<void*>(((uintptr_t)memory + ALLOCATION_HEADER_SIZE + alignment_size - 1)
                & ~(uintptr_t)(alignment_size - 1));
    } else {
        aligned_mem = reinterpret_cast<void*>((uintptr_t)memory + ALLOCATION_HEADER_SIZE);
    }

    header = reinterpret_cast<uintptr_t*>((uintptr_t)aligned_mem + ALLOCATION_HEADER_SIZE);
    *header = reinterpret_cast<uintptr_t>(memory);

    *out = aligned_mem;
cleanup:
    return status;
}

common::result environment::free(void* memory) noexcept {
    if (nullptr == memory) {
        return hype::result::SUCCESS;
    }

    uintptr_t* header = reinterpret_cast<uintptr_t*>((uintptr_t)memory + ALLOCATION_HEADER_SIZE);
    memory = reinterpret_cast<void*>(*header);

    return hype::g_context->environment.get_efi_service().free(memory);
}

uintn_t environment::to_physical(void* address) noexcept {
    // 1-to-1 mapping because of UEFI
    return reinterpret_cast<uintn_t>(address);
}
