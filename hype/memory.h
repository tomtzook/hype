#pragma once

#include <x86/paging/ia32e.h>
#include <x86/mtrr.h>
#include <x86/vmx/ept.h>
#include <x86/segments.h>

#include "base.h"

namespace hype::memory {

enum class memory_type_t {
    code,
    data
};

struct gdt_t {
    static constexpr size_t code_descriptor_index = 1;
    static constexpr size_t data_descriptor_index = 2;
    static constexpr size_t tss_descriptor_index = 3;

    x86::segments::descriptor_t null;
    x86::segments::descriptor_t code;
    x86::segments::descriptor_t data;
    x86::segments::descriptor64_t tr;
};

struct page_table_t {
    page_aligned x86::paging::ia32e::pml4e_t m_pml4;
    page_aligned x86::paging::ia32e::pdpte_t m_pdpt[x86::paging::ia32e::pdptes_in_pdpt];
};

struct ept_t {
    page_aligned x86::vmx::pml4e_t m_pml4;
    page_aligned x86::vmx::pdpte_t m_pdpt[x86::vmx::pdptes_in_pdpt];
    page_aligned x86::vmx::pde_t m_pd[x86::vmx::pdptes_in_pdpt][x86::vmx::pdes_in_directory];
};

void trace_gdt(const x86::segments::gdtr_t& gdtr);

// also loads the gdt
status_t setup_initial_guest_gdt();

status_t setup_gdt(x86::segments::gdtr_t& gdtr, gdt_t& gdt, x86::segments::tss64_t& tss);
status_t setup_identity_paging(page_table_t& page_table);
status_t setup_identity_ept(ept_t& ept, const x86::mtrr::mtrr_cache_t& mtrr_cache);

status_t load_page_table(page_table_t& page_table);

status_t allocate(void*& out, size_t size, size_t alignment, memory_type_t memory_type);
void free(void* ptr);

}
