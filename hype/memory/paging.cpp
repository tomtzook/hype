
#include <environment.h>

#include "paging.h"

namespace hype::memory {

framework::result<> setup_identity_paging(page_table_t& page_table) {
    {
        auto& pml4e = page_table.m_pml4[0];
        pml4e.raw = 0;
        pml4e.bits.present = true;
        pml4e.bits.rw = true;
        pml4e.address(environment::to_physical(page_table.m_pdpt));
    }

    for (size_t i = 0; i < x86::paging::ia32e::pdptes_in_pdpt; i++) {
        auto& pdpte = page_table.m_pdpt[i];
        pdpte.raw = 0;
        pdpte.small.present = true;
        pdpte.small.rw = true;
        pdpte.small.us = false;
        pdpte.address(environment::to_physical(page_table.m_pd[i]));

        for (size_t j = 0; j < x86::paging::ia32e::pdes_in_directory; j++) {
            auto& pde = page_table.m_pd[i][j];
            pde.raw = 0;
            pde.large.present = true;
            pde.large.rw = true;
            pde.large.ps = true;
            pde.large.us = false;
            const auto address = (i * 512 + j) * x86::paging::page_size_2m;
            pde.address(address);
        }
    }

    return {};
}

framework::result<> load_page_table(const page_table_t& page_table) {
    x86::cr3_t cr3(0);
    cr3.ia32e.address = environment::to_physical(page_table.m_pml4) >> x86::paging::page_bits_4k;
    x86::write(cr3);

    return {};
}

}
