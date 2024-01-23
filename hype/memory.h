#pragma once

#include "base.h"
#include "x86/paging/ia32e.h"

namespace hype::memory {

enum class memory_type_t {
    code,
    data
};

struct page_table_t {
public:
    const void* base_address() const noexcept;

    x86::paging::ia32e::pml4e_t m_pml4 page_aligned;
    x86::paging::ia32e::pdpte_t m_pdpt[x86::paging::ia32e::pdptes_in_pdpt] page_aligned;
};

status_t setup_identity_paging(page_table_t& page_table) noexcept;

status_t allocate(void*& out, size_t size, size_t alignment, memory_type_t memory_type) noexcept;
void free(void* ptr) noexcept;

}
