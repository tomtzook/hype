#pragma once

#include <error.h>

#include "x86/memory.h"
#include "x86/cr.h"


namespace x86::vmx {

// format for VMCS [SDM 3 24.2 P1054]
struct vmcs_t {
    uint32_t revision : 31;
    uint32_t shadow_indicator : 1;
    uint32_t abort_indicator;
    uint8_t data[x86::PAGE_SIZE - 8];
};

// format for vmxon region [SDM 3 24.11.5 P1079]
// size = vmcs size
using vmxon_region_t = vmcs_t;

bool is_supported() noexcept;

uintn_t get_cr0_fixed_bits(bool for_unrestricted_guest = false) noexcept;
void adjust_cr0_fixed_bits(x86::cr0_t& cr, bool for_unrestricted_guest = false) noexcept;

uintn_t get_cr4_fixed_bits() noexcept;
void adjust_cr4_fixed_bits(x86::cr4_t& cr) noexcept;

static_assert(sizeof(vmcs_t) <= x86::PAGE_SIZE, "vmcs_t > PAGE_SIZE");
static_assert(sizeof(vmxon_region_t) <= x86::PAGE_SIZE, "vmxon_region_t > PAGE_SIZE");

}
