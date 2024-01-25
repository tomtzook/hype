#pragma once

#include <x86/paging/ia32e.h>
#include <x86/vmx/ept.h>

#include "base.h"

namespace hype::memory {

enum class memory_type_t {
    code,
    data
};

struct page_table_t {
    x86::paging::ia32e::pml4e_t m_pml4 page_aligned;
    x86::paging::ia32e::pdpte_t m_pdpt[x86::paging::ia32e::pdptes_in_pdpt] page_aligned;
};

struct ept_t {
    x86::vmx::pml4e_t m_pml4 page_aligned;
    x86::vmx::pdpte_t m_pdpt[x86::vmx::pdptes_in_pdpt] page_aligned;
    x86::vmx::pde_t m_pd[x86::vmx::pdptes_in_pdpt][x86::vmx::pdes_in_directory] page_aligned;
};

status_t setup_identity_paging(page_table_t& page_table) noexcept;

status_t allocate(void*& out, size_t size, size_t alignment, memory_type_t memory_type) noexcept;
void free(void* ptr) noexcept;

}
