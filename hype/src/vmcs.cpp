
#include <x86/cr.h>

#include "vmcs.h"


static void setup_cr_dr_vmcs(x86::vmx::vmcs_t& vmcs) noexcept {
    {
        x86::cr0_t cr0;
        vmcs.vmwrite(x86::vmx::VMCS_HOST_CR0, cr0.raw);

        x86::vmx::adjust_cr0_fixed_bits(cr0, true);
        vmcs.vmwrite(x86::vmx::VMCS_GUEST_CR0, cr0.raw);
        vmcs.vmwrite(x86::vmx::VMCS_CTRL_CR0_READ_SHADOW, cr0.raw);
        vmcs.vmwrite(x86::vmx::VMCS_CTRL_CR0_GUEST_HOST_MASK, hype::get_cr0_host_mask());
    }

    {
        x86::cr4_t cr4;
        vmcs.vmwrite(x86::vmx::VMCS_HOST_CR0, cr4.raw);

        x86::vmx::adjust_cr4_fixed_bits(cr4);
        vmcs.vmwrite(x86::vmx::VMCS_GUEST_CR4, cr4.raw);

        cr4.bits.vmx_enable = false; // hide that vmx is enabled
        vmcs.vmwrite(x86::vmx::VMCS_CTRL_CR4_READ_SHADOW, cr4.raw);
        vmcs.vmwrite(x86::vmx::VMCS_CTRL_CR4_GUEST_HOST_MASK, hype::get_cr4_host_mask());
    }
}

static void setup_segments_vmcs(x86::vmx::vmcs_t& vmcs) noexcept {

}

uintn_t hype::get_cr0_host_mask() noexcept {
    x86::cr0_t cr0(0);
    cr0.bits.paging_enable = true;

    return cr0.raw;
}

uintn_t hype::get_cr4_host_mask() noexcept {
    x86::cr4_t cr4(0);

    return cr4.raw;
}

common::result hype::setup_vmcs(hype::vcpu_t& cpu, x86::vmx::vmcs_t& vmcs) noexcept {
    setup_cr_dr_vmcs(vmcs);
    setup_segments_vmcs(vmcs);

    return common::result::SUCCESS;
}
