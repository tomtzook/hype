#pragma once

#include <x86/vmx/ept.h>
#include <base.h>

namespace hype::memory {

#pragma pack(push, 1)

struct ept_t {
    page_aligned x86::vmx::pml4e_t m_pml4[x86::vmx::pml4e_in_pml4];
    page_aligned x86::vmx::pdpte_t m_pdpt[x86::vmx::pdptes_in_pdpt];
    page_aligned x86::vmx::pde_t m_pd[x86::vmx::pdptes_in_pdpt][x86::vmx::pdes_in_directory];
};

#pragma pack(pop)

framework::result<> setup_identity_ept(ept_t& ept, const x86::mtrr::mtrr_cache_t& mtrr_cache);

}
