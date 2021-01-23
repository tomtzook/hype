
#include "x86/cpuid.h"

#include "x86/vmx/msr.h"

#include "x86/vmx/environment.h"


bool x86::vmx::is_supported() noexcept {
    // CPUID.1:ECX.VMX[bit 5] = 1 [SDM 3 23.6 P1050]
    x86::cpuid_eax01_t cpu_features{};
    x86::cpu_id(1, cpu_features);

    if(!cpu_features.ecx.bits.vmx) {
        return false;
    }

    return true;
}

uintn_t x86::vmx::get_cr0_fixed_bits(bool for_unrestricted_guest) noexcept {
    // [SDM 3 A.7 P1960]
    // F0[X] = 1 -> must be 1
    // F1[X] = 0 -> must be 0
    uint64_t fixed0 = x86::msr::read(x86::msr::ia32_vmx_cr0_fixed0_t::ID);
    uint64_t fixed1 = x86::msr::read(x86::msr::ia32_vmx_cr0_fixed1_t::ID);

    if (for_unrestricted_guest) {
        // when in unrestricted guest mode, we don't need to account
        // for PG or PE bits
        fixed1 &= ~(CR0_PG_MASK | CR0_PE_MASK);
    }

    return fixed0 & fixed1;
}

void x86::vmx::adjust_cr0_fixed_bits(x86::cr0_t& cr, bool for_unrestricted_guest) noexcept {
    cr.raw |= get_cr0_fixed_bits(for_unrestricted_guest);
}

uintn_t x86::vmx::get_cr4_fixed_bits() noexcept {
    // [SDM 3 A.8 P1960]
    uint64_t fixed0 = x86::msr::read(x86::msr::ia32_vmx_cr4_fixed0_t::ID);
    uint64_t fixed1 = x86::msr::read(x86::msr::ia32_vmx_cr4_fixed1_t::ID);

    return fixed0 & fixed1;
}

void x86::vmx::adjust_cr4_fixed_bits(x86::cr4_t& cr) noexcept {
    cr.raw |= get_cr4_fixed_bits();
}

common::result x86::vmx::initialize_vm_struct(x86::vmx::vm_struct_t& vm_struct) noexcept {
    common::result status;

    // check the size according to IA32_VMX_BASIC [SDM 3 24.11.5 P1079]
    x86::msr::ia32_vmx_basic_t vmx_basic {};
    x86::msr::read(x86::msr::ia32_vmx_basic_t::ID, vmx_basic);

    CHECK_ASSERT(sizeof(vm_struct) >= vmx_basic.bits.vm_struct_size,
                 "vm struct size too small");

    // initialize the revision indicator
    vm_struct.revision = vmx_basic.bits.vmcs_revision;
    vm_struct.shadow_indicator = false;

cleanup:
    return status;
}
