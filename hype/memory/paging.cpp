
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

#ifdef paging_level_4k
            pde.large.ps = false;
            pde.small.present = true;
            pde.small.rw = true;
            pde.small.us = false;

            pde.address(environment::to_physical(page_table.m_pt[i][j]));

            for (size_t k = 0; k < x86::paging::ia32e::ptes_in_table; k++) {
                auto& pte = page_table.m_pt[i][j][k];
                pte.raw = 0;
                pte.bits.present = true;
                pte.bits.rw = true;
                pte.bits.us = false;

                const auto address = (i * 512 * 512 + j * 512 + k) * x86::paging::page_size_4k;
                pte.address(address);
            }
#else
            pde.large.ps = true;
            pde.large.present = true;
            pde.large.rw = true;
            pde.large.us = false;

            const auto address = (i * 512 + j) * x86::paging::page_size_2m;
            pde.address(address);
#endif
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

framework::result<> split_large_into_small(page_table_t& page_table, const x86::paging::ia32e::linear_address_t address) {
    if (page_table.current_pt_index >= x86::paging::ia32e::pdes_in_directory) {
        return framework::err(framework::status_assert_failed);
    }

    auto& pde = page_table.m_pd[address.large.directory_pointer][address.large.directory];
    auto& pt = page_table.m_pt_for_split[page_table.current_pt_index++];

    const auto start_address = pde.address();
    for (int i = 0; i < x86::paging::ia32e::pdes_in_directory; i++) {
        auto& pte = pt[i];
        pte.raw = 0;
        pte.bits.present = true;
        pte.bits.rw = pde.large.rw;
        pte.bits.xd = pde.large.xd;
        pte.bits.us = false;
        pte.address(start_address + i * x86::paging::page_size_4k);
    }

    pde.large.ps = false;
    pde.address(environment::to_physical(pt));

    return {};
}

}
