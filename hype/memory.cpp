
#include <x86/paging/paging.h>
#include <x86/paging/ia32e.h>

#include "base.h"
#include "environment.h"
#include "memory.h"


namespace hype::memory {

struct allocation_header_t {
    uintptr_t ptr;
    size_t pages;
} packed;

status_t setup_identity_paging(page_table_t& page_table) noexcept {
    CHECK_ASSERT(x86::paging::current_mode() == x86::paging::mode_t::ia32e,
                 "paging mode is not ia32e");
    CHECK_ASSERT(x86::paging::ia32e::are_huge_tables_supported(),
                 "paging huge tables not supported");

    {
        auto& pml4e = page_table.m_pml4;
        pml4e.raw = 0;
        pml4e.bits.present = true;
        pml4e.bits.rw = true;
        pml4e.address(environment::to_physical(page_table.m_pdpt));
    }

    for(size_t i = 0; i < x86::paging::ia32e::pdptes_in_pdpt; i++) {
        auto& pdpte = page_table.m_pdpt[i];
        pdpte.raw = 0;
        pdpte.huge.present = true;
        pdpte.huge.rw = true;
        pdpte.huge.ps = true;
        pdpte.address(i * x86::paging::page_size_1g);
    }

    return {};
}

status_t allocate(void*& out, size_t size, size_t alignment, memory_type_t memory_type) noexcept {
    void* memory;
    size_t pages = (size + sizeof(allocation_header_t) + alignment - 1) / x86::paging::page_size;
    CHECK(environment::allocate_pages(memory, pages, memory_type));

    void* aligned_mem;
    if (0 != alignment) {
        aligned_mem = reinterpret_cast<void*>((reinterpret_cast<uintptr_t>(memory) +
                sizeof(allocation_header_t)  + alignment - 1) & ~(uintptr_t)(alignment - 1));
    } else {
        aligned_mem = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(memory) + sizeof(allocation_header_t));
    }

    auto header = reinterpret_cast<allocation_header_t*>(reinterpret_cast<uintptr_t>(aligned_mem) - sizeof(allocation_header_t));
    header->ptr = reinterpret_cast<uintptr_t>(memory);
    header->pages = pages;

    out = aligned_mem;
    return {};
}

void free(void* ptr) noexcept {
    auto header = reinterpret_cast<allocation_header_t*>(reinterpret_cast<uintptr_t>(ptr) - sizeof(allocation_header_t));
    environment::free_pages(reinterpret_cast<void*>(header->ptr), header->pages);
}

}
