#pragma once

#include <x86/paging/ia32e.h>

#include <common/common.h>


namespace hype::environment {

class page_table {
public:
    const void* base_address() const noexcept;

    static result::status setup_identity_paging(page_table& page_table) noexcept;
private:
    x86::paging::ia32e::pml4e_t m_pml4 page_aligned;
    x86::paging::ia32e::pdpte_t m_pdpt[x86::paging::ia32e::pdptes_in_pdpt] page_aligned;
};

bool are_huge_tables_supported() noexcept;

}
