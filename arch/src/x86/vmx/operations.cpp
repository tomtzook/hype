
#include <environment.h>
#include <crt.h>

#include "x86/error.h"
#include "x86/cpuid.h"
#include "x86/cr.h"
#include "x86/msr.h"
#include "x86/intrinsic.h"
#include "x86/memory.h"

#include "x86/vmx/instrinsic.h"

#include "x86/vmx/operations.h"


static common::result prepare_for_on() {
    // check IA32_FEATURE_CONTROL MSR [SDM 3 23.7 P1051]
    //      bit[0] = 0 -> vmxon #GP
    //      bit[1] = 0 -> vmxon [SMX mode] #GP
    //      bit[2] = 0 -> vmxon [non-SMX mode] #GP
    // SMX support CPUID.1:ECX[6] = 1 [SDM 2 6.2.1]
    //      SMX enabled CR4.SMXE[14] = 1
    common::result status = common::result::SUCCESS;

    x86::msr::ia32_feature_ctrl_t feature_ctrl {};
    x86::msr::read(x86::msr::ia32_feature_ctrl_t::ID, feature_ctrl);

    x86::cr4_t cr4;

    if (!feature_ctrl.bits.lock_bit) {
        // lock bit is off, so we set what we need and lock
        if (cr4.bits.smx_enable) {
            feature_ctrl.bits.vmx_smx = true;
        } else {
            feature_ctrl.bits.vmx_no_smx = true;
        }
        feature_ctrl.bits.lock_bit = true;
        x86::msr::write(x86::msr::ia32_feature_ctrl_t::ID, feature_ctrl);
    } else if ((cr4.bits.smx_enable && !feature_ctrl.bits.vmx_smx) ||
               (!cr4.bits.smx_enable && !feature_ctrl.bits.vmx_no_smx)) {
        // ia32_feature_ctrl does not support the current SMX mode
        ERROR(x86::result::STATE_NOT_SUPPORTED);
    }

    // enable vmx CR4.VMXE[13] = 1 [SDM 3 23.7 P1051]
    cr4.bits.vmx_enable = 1;
    x86::write(cr4);

cleanup:
    return status;
}


common::result x86::vmx::on(void*& vmxon_region) noexcept {
    common::result status = common::result::SUCCESS;
    x86::msr::ia32_vmx_basic_t vmx_basic {};
    uint64_t vmxon_region_size;
    uintn_t vmxon_physaddr;

    CHECK(prepare_for_on());

    // loading vmxon region required [SDM 3 24.11.5 P1079]
    //      vmxon region size determined from IA32_VMX_BASIC MSR
    //      vmxon address must be page aligned
    //      vmxon address must not exceed maxphysaddr
    x86::msr::read(x86::msr::ia32_vmx_basic_t::ID, vmx_basic);

    vmxon_region_size = vmx_basic.bits.vm_struct_size;
    vmxon_region = new (environment::alignment_t::PAGE_ALIGN) uint8_t[vmxon_region_size];
    CHECK_ALLOCATION(vmxon_region);

    vmxon_physaddr = environment::to_physical(&vmxon_region);
    CHECK_ASSERT(x86::is_page_aligned(vmxon_physaddr), "vmxon region not page aligned");
    CHECK_ASSERT(x86::does_address_in_max_physical_width(vmxon_physaddr),
                 "vmxon region exceeds maxphysaddress");

    vmxon(vmxon_physaddr);

cleanup:
    return status;
}
