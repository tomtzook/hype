
#include "environment.h"

#include "vmx.h"


namespace hype {

static uintn_t get_cr0_host_mask() noexcept {
    x86::cr0_t cr0(0);
    cr0.bits.paging_enable = true;

    return cr0.raw;
}

static uintn_t get_cr4_host_mask() noexcept {
    x86::cr4_t cr4(0);

    return cr4.raw;
}

static void setup_cr_dr_vmcs() noexcept {
    {
        auto cr0 = x86::read<x86::cr0_t>();
        x86::vmx::vmwrite(x86::vmx::field_t::host_cr0, cr0.raw);

        x86::vmx::adjust_cr0_fixed_bits(cr0, true);
        x86::vmx::vmwrite(x86::vmx::field_t::guest_cr0, cr0.raw);
        x86::vmx::vmwrite(x86::vmx::field_t::ctrl_cr0_read_shadow, cr0.raw);
        x86::vmx::vmwrite(x86::vmx::field_t::ctrl_cr0_guest_host_mask, get_cr0_host_mask());
    }

    {
        auto cr4 = x86::read<x86::cr4_t>();
        x86::vmx::vmwrite(x86::vmx::field_t::host_cr4, cr4.raw);

        x86::vmx::adjust_cr4_fixed_bits(cr4);
        cr4.bits.vmx_enable = false; // hide that vmx is enabled
        x86::vmx::vmwrite(x86::vmx::field_t::guest_cr4, cr4.raw);
        x86::vmx::vmwrite(x86::vmx::field_t::ctrl_cr4_read_shadow, cr4.raw);
        x86::vmx::vmwrite(x86::vmx::field_t::ctrl_cr4_guest_host_mask, get_cr4_host_mask());
    }
}

static void setup_segments_vmcs() noexcept {

}

status_t vmxon_for_vcpu(vcpu_t& cpu) noexcept {
    CHECK_ASSERT(x86::vmx::prepare_for_vmxon(), "prepare_for_vmxon failed");
    CHECK_ASSERT(x86::vmx::initialize_vmstruct(cpu.vmxon_region), "initialize_vmstruct failed");

    auto vmxon_region = environment::to_physical(&cpu.vmxon_region);
    CHECK_VMX(x86::vmx::vmxon(vmxon_region));

    return {};
}

status_t setup_vmcs(vcpu_t& cpu) noexcept {
    setup_cr_dr_vmcs();
    setup_segments_vmcs();

    return {};
}

}
