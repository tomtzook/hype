
#include <crt.h>

#include "x86/error.h"
#include "x86/cpuid.h"
#include "x86/cr.h"
#include "x86/memory.h"

#include "x86/vmx/instrinsic.h"
#include "x86/vmx/environment.h"
#include "x86/vmx/msr.h"
#include "x86/vmx/vmcs.h"

#include "x86/vmx/operations.h"


static x86::vmx::vmcs_t* g_loaded_vmcs = nullptr;


static common::result prepare_for_on() {
    // check IA32_FEATURE_CONTROL MSR [SDM 3 23.7 P1051]
    //      bit[0] = 0 -> vmxon #GP
    //      bit[1] = 0 -> vmxon [SMX mode] #GP
    //      bit[2] = 0 -> vmxon [non-SMX mode] #GP
    // SMX support CPUID.1:ECX[6] = 1 [SDM 2 6.2.1]
    //      SMX enabled CR4.SMXE[14] = 1
    common::result status;

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
        ERROR(common::result::UNSUPPORTED_ERROR);
    }

    {
        // Restrictions placed on CR0 and CR4 [SDM 3 23.8 P1051]
        x86::cr0_t cr0;
        x86::vmx::adjust_cr0_fixed_bits(cr0);
        x86::write(cr0);

        x86::vmx::adjust_cr4_fixed_bits(cr4);
        // enable vmx CR4.VMXE[13] = 1 [SDM 3 23.7 P1051]
        cr4.bits.vmx_enable = 1;
        x86::write(cr4);
    }

cleanup:
    return status;
}

common::result x86::vmx::on(vmxon_region_t& vmxon_region) noexcept {
    common::result status;

    CHECK(prepare_for_on());
    CHECK(initialize_vm_struct(vmxon_region));

    {
        // loading vmxon region required [SDM 3 24.11.5 P1079]
        //      vmxon region size determined from IA32_VMX_BASIC MSR
        //          however, it will never acceed PAGE_SIZE, so we will receive
        //          an allocated PAGE_SIZE data.
        //      vmxon address must be page aligned
        //      vmxon address must not exceed maxphysaddr
        physical_address_t vmxon_physical = environment::to_physical(&vmxon_region);
        CHECK_ASSERT(x86::is_page_aligned(vmxon_physical), "vmxon region not page aligned");
        CHECK_ASSERT(x86::is_address_in_max_physical_width(vmxon_physical),
                     "vmxon region exceeds maxphysaddress");

        CHECK(_vmxon(vmxon_physical));
    }

cleanup:
    return status;
}

common::result x86::vmx::off() noexcept {
    return _vmxoff();
}

common::result x86::vmx::read(vmcs_t& vmcs) noexcept {
    common::result status;

    CHECK(_vmptrst(&vmcs));

cleanup:
    return status;
}

common::result x86::vmx::write(vmcs_t& vmcs) noexcept {
    common::result status;

    if (g_loaded_vmcs == &vmcs || vmcs.m_loaded) {
        goto cleanup;
    }

    CHECK(_vmptrld(environment::to_physical(&vmcs)));
    vmcs.m_loaded = true;
    if (nullptr != g_loaded_vmcs) {
        g_loaded_vmcs->m_loaded = false;
    }
    g_loaded_vmcs = &vmcs;

cleanup:
    return status;
}

common::result x86::vmx::launch() noexcept {
    return x86::vmx::_vmlaunch();
}

common::result x86::vmx::resume() noexcept {
    return common::result::UNSUPPORTED_ERROR;
}
