
#include <x86/cpuid.h>

#include "memory.h"
#include "paging.h"


namespace hype::environment {

static constexpr x86::cpuid_t paging_features_leaf = 0x80000001;

const void* page_table::base_address() const noexcept {
    return &m_pml4;
}

result::status page_table::setup_identity_paging(page_table& page_table) noexcept {
    CHECK_ASSERT(x86::paging::current_mode() == x86::paging::mode_t::ia32e, "paging mode is not ia32e");
    CHECK_ASSERT(are_huge_tables_supported(), "huge pages not supported");

    using namespace x86::paging::ia32e;
    {
        auto pdpt_physical = environment::to_physical(page_table.m_pdpt);
        pml4e_t& pml4e = page_table.m_pml4;
        pml4e.raw = 0;
        pml4e.bits.present = true;
        pml4e.bits.rw = true;
        pml4e.address(pdpt_physical);
    }

    for (size_t i = 0; i < pdptes_in_pdpt; i++) {
        auto& pdpte = page_table.m_pdpt[i];
        pdpte.raw = 0;
        pdpte.huge.present = true;
        pdpte.huge.rw = true;
        pdpte.huge.ps = true;
        pdpte.address((i * x86::paging::page_size_1g));
    }

    return {};
}

bool are_huge_tables_supported() noexcept {
    // CPUID[0x80000001].EDX[26] = 1 -> 1gb pages supported [SDM 3 4.1.4 P109]
    auto regs = x86::cpuid<paging_features_leaf>();
    return CHECK_BIT(regs.edx, 26);
}

}
