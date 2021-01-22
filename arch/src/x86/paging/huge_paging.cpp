
#include <macros.h>
#include <environment.h>
#include <types.h>

#include "x86/error.h"
#include "x86/cr.h"
#include "x86/msr.h"
#include "x86/cpuid.h"
#include "x86/memory.h"

#include "x86/paging/huge_paging.h"


static constexpr x86::cpuid_t PAGING_FEATURES_LEAF = 0x80000001;
static constexpr size_t SIZE_1GB_SHIFT = 30;


physical_address_t x86::paging::huge_pdpte_t::page_address() const noexcept {
    return static_cast<physical_address_t>(bits.page_address) << SIZE_1GB_SHIFT;
}
void x86::paging::huge_pdpte_t::page_address(physical_address_t address) noexcept {
    bits.page_address = address >> SIZE_1GB_SHIFT;
}

physical_address_t x86::paging::pml4e_t::page_directory_address() const noexcept {
    return PAGE_SHIFT_LEFT(static_cast<physical_address_t>(bits.page_directory_address));
}
void x86::paging::pml4e_t::page_directory_address(physical_address_t address) noexcept {
    bits.page_directory_address = PAGE_SHIFT_RIGHT(address);
}

x86::paging::huge_paging_virtual_address_t::huge_paging_virtual_address_t(const void* address) noexcept
    : m_address(reinterpret_cast<const uintptr_t>(address))
{}
x86::paging::huge_paging_virtual_address_t::huge_paging_virtual_address_t(const uintptr_t address) noexcept
    : m_address(address)
{}

size_t x86::paging::huge_paging_virtual_address_t::pml4e_offset() const noexcept {
    return EXTRACT_BITS(m_address, 39, 0x1ff);
}
size_t x86::paging::huge_paging_virtual_address_t::pdpte_offset() const noexcept {
    return EXTRACT_BITS(m_address, 30, 0xff);
}
size_t x86::paging::huge_paging_virtual_address_t::page_offset() const noexcept {
    return EXTRACT_BITS(m_address, 0, 0x1fffffff);
}


const void* x86::paging::huge_page_table_t::base_address() const noexcept {
    return reinterpret_cast<const void*>(&m_pml4);
}

physical_address_t x86::paging::huge_page_table_t::to_physical(
        const huge_paging_virtual_address_t& address
        ) const noexcept {
    // IA-32E huge paging translation [SDM 3 4.5 P125 "Figure 4-10"]
    if (0 != address.pml4e_offset()) {
        return PHYSICAL_NULL_ADDRESS;
    }

    const huge_pdpte_t& pdpte = m_pdpt[address.pdpte_offset()];
    return pdpte.page_address() + address.page_offset();
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
        physical_address_t pdpt_physical = environment::to_physical(page_table.m_pdpt);
        pml4e_t& pml4e = page_table.m_pml4;
        pml4e.raw = 0;
        pml4e.bits.present = true;
        pml4e.bits.read_write = true;
        pml4e.page_directory_address(pdpt_physical);
    }

    for (size_t i = 0; i < huge_page_table_t::PDPT_ENTRIES_COUNT; i++) {
        huge_pdpte_t& pdpte = page_table[i];
        pdpte.raw = 0;
        pdpte.bits.present = true;
        pdpte.bits.read_write = true;
        pdpte.bits.page_size = true;
        pdpte.page_address((i * SIZE_1GB));
    }

cleanup:
    return status;
}
