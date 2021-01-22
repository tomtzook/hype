
#include <x86/msr.h>
#include <x86/cr.h>

#include "x86/paging/paging.h"


x86::paging::mode_t x86::paging::get_mode() noexcept {
    // PAE paging mode
    //  CR0.PG = 1, CR4.PAE = 1, IA32_EFER.LME = 0
    // IA32e paging mode [SDM 3 4.1.1 P106]
    //  CR0.PG = 1, CR4.PAE = 1, IA32_EFER.LME = 1
    cr0_t cr0;
    cr4_t cr4;
    msr::ia32_efer_t efer {};

    if (!efer.bits.lme) {
        if (cr0.bits.paging_enable && !cr4.bits.physical_address_extension) {
            return mode_t::PAE_PAGING;
        } else {
            return mode_t::BIT32_PAGING;
        }
    } else {
        // IA32e
        return mode_t::IA32E_PAGING;
    }
}
