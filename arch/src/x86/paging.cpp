
#include <macros.h>
#include <environment.h>

#include "x86/error.h"
#include "x86/cr.h"
#include "x86/msr.h"
#include "x86/cpuid.h"
#include "x86/memory.h"

#include "x86/paging.h"


static const x86::cpuid_t PAGING_FEATURES_LEAF = 0x80000001;


const void* x86::paging::huge_page_table_t::base_address() const noexcept {
    return reinterpret_cast<const void*>(&m_pml4);
}

physical_address_t x86::paging::huge_page_table_t::to_physical(const void* memory) const noexcept {
    // IA-32E huge paging translation [SDM 3 4.5 P125 "Figure 4-10"]
    // TODO: make a class for virtual address and use it to translate
    const uintn_t address = reinterpret_cast<const uintn_t>(memory);
    const uintn_t pml4e_idx = EXTRACT_BITS(address, 39, 0x1ff);
    const uintn_t pdpte_idx = EXTRACT_BITS(address, 30, 0xff);
    const uintn_t offset = EXTRACT_BITS(address, 0, 0x1fffffff);

    if (0 != pml4e_idx) {
        // problem, we don't support this yet...
    }

    const huge_pdpte_t& pdpte = m_pdpt[pdpte_idx];
    return (static_cast<physical_address_t>(pdpte.page_address) << 32) + offset;
}

x86::paging::pml4e_t& x86::paging::huge_page_table_t::pml4() noexcept {
    return m_pml4;
}

x86::paging::huge_pdpte_t& x86::paging::huge_page_table_t::operator[](size_t index) noexcept {
    return m_pdpt[index];
}

bool x86::paging::are_huge_tables_supported() noexcept {
    // IA32e paging mode [SDM 3 4.1.1 P106]
    //  CR0.PG = 1, CR4.PAE = 1, IA32_EFER.LME = 1
    // CPUID[0x80000001].EDX[26] = 1 -> 1gb pages supported [SDM 3 4.1.4 P109]
    cr0_t cr0;
    cr4_t cr4;
    msr::ia32_efer_t efer {};

    if (!cr0.bits.paging_enable ||
            !cr4.bits.physical_address_extension ||
            !efer.bits.lme) {
        return false;
    }

    cpuid_regs_t cpuid_regs {};
    cpu_id(PAGING_FEATURES_LEAF, cpuid_regs);

    if (CHECK_BIT(cpuid_regs.edx, 26)) {
        return false;
    }

    return true;
}

common::result x86::paging::setup_identity_paging(huge_page_table_t& page_table) noexcept {
    common::result status = x86::result::SUCCESS;

    CHECK_ASSERT(are_huge_tables_supported(), "huge pages not supported");

    {
        uintn_t pdpt_physical = environment::to_physical(&page_table[0]);
        pml4e_t& pml4e = page_table.pml4();
        pml4e.raw = 0;
        pml4e.present = true;
        pml4e.read_write = true;
        pml4e.page_directory_address = PAGE_SHIFT_RIGHT(pdpt_physical);
    }

    for (size_t i = 0; i < PDPT_ENTRIES_COUNT; i++) {
        huge_pdpte_t& pdpte = page_table[i];
        pdpte.raw = 0;
        pdpte.present = true;
        pdpte.read_write = true;
        pdpte.page_size = true;
        pdpte.page_address = (i * SIZE_1GB) >> 30;
    }

cleanup:
    return status;
}
