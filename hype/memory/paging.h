#pragma once

#include <x86/paging/ia32e.h>
#include <x86/paging/paging.h>

#include <base.h>

namespace hype::memory {

//#define paging_level_4k 1

#pragma pack(push, 1)

struct page_table_t {
    static constexpr size_t stack_guard_pml4e = 5;
    static constexpr size_t guest_mapping_pml4e = 10;

    page_aligned x86::paging::ia32e::pml4e_t m_pml4[x86::paging::ia32e::pml4e_in_pml4];
    page_aligned x86::paging::ia32e::pdpte_t m_pdpt[x86::paging::ia32e::pdptes_in_pdpt];
    page_aligned x86::paging::ia32e::pde_t m_pd[x86::paging::ia32e::pdptes_in_pdpt][x86::paging::ia32e::pdes_in_directory];
#ifdef paging_level_4k
    page_aligned x86::paging::ia32e::pte_t m_pt[x86::paging::ia32e::pdptes_in_pdpt][x86::paging::ia32e::pdes_in_directory][x86::paging::ia32e::ptes_in_table];
#else
    page_aligned x86::paging::ia32e::pte_t m_pt_for_split[x86::paging::ia32e::pdes_in_directory][x86::paging::ia32e::ptes_in_table];
    size_t current_pt_index = 0;
#endif
};

#pragma pack(pop)

framework::result<> setup_identity_paging(page_table_t& page_table);
framework::result<> load_page_table(const page_table_t& page_table);
framework::result<> split_large_into_small(page_table_t& page_table, x86::paging::ia32e::linear_address_t address);

}
