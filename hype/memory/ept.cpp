
#include <environment.h>

#include "ept.h"

namespace hype::memory {

framework::result<> setup_identity_ept(ept_t& ept, const x86::mtrr::mtrr_cache_t& mtrr_cache) {
    {
        auto& pml4e = ept.m_pml4[0];
        pml4e.raw = 0;
        pml4e.bits.read = true;
        pml4e.bits.write = true;
        pml4e.bits.execute = true;
        pml4e.address(environment::to_physical(ept.m_pdpt));
    }

    for (int i = 0; i < x86::vmx::pdptes_in_pdpt; i++) {
        auto& pdpte = ept.m_pdpt[i];
        pdpte.raw = 0;
        pdpte.small.read = true;
        pdpte.small.write = true;
        pdpte.small.execute = true;
        pdpte.address(environment::to_physical(ept.m_pd[i]));

        for (int j = 0; j < x86::vmx::pdes_in_directory; j++) {
            auto& pde = ept.m_pd[i][j];
            pde.raw = 0;
            pde.large.read = true;
            pde.large.write = true;
            pde.large.execute = true;
            pde.large.ps = true;

            const auto address = (i * 512 + j) * x86::paging::page_size_2m;
            auto type = mtrr_cache.type_for_2m(address);
            assert(x86::mtrr::memory_type_invalid != type, "mtrr for range is invalid");

            pde.large.mem_type = static_cast<uint64_t>(type);
            pde.address(address);
        }
    }

    return {};
}

}
