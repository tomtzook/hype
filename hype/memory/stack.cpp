
#include "stack.h"


namespace hype::memory {

void stack_guard::map_into_pml4e(page_table_t& table, const size_t pml4e_index) {
    m_pml4e_index = pml4e_index;
    map_into_pml4e(table.m_pml4[pml4e_index]);
}

void stack_guard::map_into_pml4e(x86::paging::ia32e::pml4e_t& pml4e) {
    pml4e.raw = 0;
    pml4e.bits.rw = true;
    pml4e.bits.present = true;
    pml4e.address(environment::to_physical(m_pdpt));

    // initialize only the first pdpte
    auto& pdpte = m_pdpt[0];
    pdpte.raw = 0;
    pdpte.small.rw = true;
    pdpte.small.present = true;
    pdpte.address(environment::to_physical(m_pd));

    // reset pde index
    m_pde_index = 0;
}

}
