
#include <x86/vmx/segments.h>
#include <x86/vmx/controls.h>
#include <x86/interrupts.h>

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

template<typename _controls>
static status_t setup_vm_controls(const _controls& controls) {
    auto adjusted = x86::vmx::adjust_vm_controls(controls);
    CHECK_VMX(x86::vmx::vmwrite(_controls::vmcs_field, adjusted.raw));

    return {};
}

template<typename _vmcs_segment_defs>
static status_t setup_segment(x86::segments::table_t& table) noexcept {
    // todo: for guest, use original efi segments
    // todo: handle null segments: set access rights as unusable

    auto segment = x86::read<typename _vmcs_segment_defs::segment>();
    if (segment.bits.index == 0) {
        x86::vmx::segment_access_rights_t ar{};
        ar.bits.unusable = true;

        CHECK_VMX(x86::vmx::vmwrite(_vmcs_segment_defs::guest_selector, 0));
        CHECK_VMX(x86::vmx::vmwrite(_vmcs_segment_defs::guest_base, 0));
        CHECK_VMX(x86::vmx::vmwrite(_vmcs_segment_defs::guest_limit, 0));
        CHECK_VMX(x86::vmx::vmwrite(_vmcs_segment_defs::guest_ar, ar.raw));
        CHECK_VMX(x86::vmx::vmwrite(_vmcs_segment_defs::host_selector, 0));

        return {};
    }

    auto descriptor = table[segment.bits.index];

    CHECK_VMX(x86::vmx::vmwrite(_vmcs_segment_defs::guest_selector, segment.value));
    CHECK_VMX(x86::vmx::vmwrite(_vmcs_segment_defs::guest_base, descriptor.base_address()));
    CHECK_VMX(x86::vmx::vmwrite(_vmcs_segment_defs::guest_limit, descriptor.limit()));
    CHECK_VMX(x86::vmx::vmwrite(_vmcs_segment_defs::guest_ar, x86::vmx::segment_access_rights(descriptor).raw));
    CHECK_VMX(x86::vmx::vmwrite(_vmcs_segment_defs::host_selector, segment.value));

    return {};
}

static status_t setup_cr_dr_vmcs() noexcept {
    {
        auto cr0 = x86::read<x86::cr0_t>();
        CHECK_VMX(x86::vmx::vmwrite(x86::vmx::field_t::host_cr0, cr0.raw));

        x86::vmx::adjust_cr0_fixed_bits(cr0, true);
        CHECK_VMX(x86::vmx::vmwrite(x86::vmx::field_t::guest_cr0, cr0.raw));
        CHECK_VMX(x86::vmx::vmwrite(x86::vmx::field_t::ctrl_cr0_read_shadow, cr0.raw));
        CHECK_VMX(x86::vmx::vmwrite(x86::vmx::field_t::ctrl_cr0_guest_host_mask, get_cr0_host_mask()));
    }

    {
        auto cr4 = x86::read<x86::cr4_t>();
        CHECK_VMX(x86::vmx::vmwrite(x86::vmx::field_t::host_cr4, cr4.raw));

        x86::vmx::adjust_cr4_fixed_bits(cr4);
        cr4.bits.vmx_enable = false; // hide that vmx is enabled
        CHECK_VMX(x86::vmx::vmwrite(x86::vmx::field_t::guest_cr4, cr4.raw));
        CHECK_VMX(x86::vmx::vmwrite(x86::vmx::field_t::ctrl_cr4_read_shadow, cr4.raw));
        CHECK_VMX(x86::vmx::vmwrite(x86::vmx::field_t::ctrl_cr4_guest_host_mask, get_cr4_host_mask()));
    }

    {
        auto cr3 = x86::read<x86::cr3_t>();

        CHECK_VMX(x86::vmx::vmwrite(x86::vmx::field_t::host_cr3, cr3.raw));
        // todo: what value
        CHECK_VMX(x86::vmx::vmwrite(x86::vmx::field_t::guest_cr3, 0));
    }

    {
        auto efer = x86::read<x86::msr::ia32_efer_t>();
        CHECK_VMX(x86::vmx::vmwrite(x86::vmx::field_t::host_efer, efer.raw));
        CHECK_VMX(x86::vmx::vmwrite(x86::vmx::field_t::guest_efer, efer.raw));
    }

    {
        auto dr7 = x86::read<x86::dr7_t>();
        CHECK_VMX(x86::vmx::vmwrite(x86::vmx::field_t::guest_dr7, dr7.raw));
    }

    {
        auto debugctl = x86::read<x86::msr::ia32_debugctl_t>();
        CHECK_VMX(x86::vmx::vmwrite(x86::vmx::field_t::guest_debugctl, debugctl.raw));
    }

    return {};
}

static status_t setup_segments_vmcs() noexcept {
    auto gdtr = x86::read<x86::segments::gdtr_t>();
    auto gdt_table = x86::segments::table_t(gdtr);

    CHECK(setup_segment<x86::vmx::vmcs_cs_segment>(gdt_table));
    CHECK(setup_segment<x86::vmx::vmcs_ds_segment>(gdt_table));
    CHECK(setup_segment<x86::vmx::vmcs_gs_segment>(gdt_table));
    CHECK(setup_segment<x86::vmx::vmcs_ss_segment>(gdt_table));
    CHECK(setup_segment<x86::vmx::vmcs_es_segment>(gdt_table));
    CHECK(setup_segment<x86::vmx::vmcs_fs_segment>(gdt_table));

    // todo: update TR and LDT

    CHECK_VMX(x86::vmx::vmwrite(x86::vmx::field_t::guest_gdtr_base, gdtr.base_address));
    CHECK_VMX(x86::vmx::vmwrite(x86::vmx::field_t::guest_gdtr_base, gdtr.limit));
    CHECK_VMX(x86::vmx::vmwrite(x86::vmx::field_t::host_gdtr_base, gdtr.base_address));

    auto idtr = x86::read<x86::interrupts::idtr_t>();
    CHECK_VMX(x86::vmx::vmwrite(x86::vmx::field_t::guest_idtr_base, idtr.base_address));
    CHECK_VMX(x86::vmx::vmwrite(x86::vmx::field_t::guest_idtr_limit, idtr.limit));
    CHECK_VMX(x86::vmx::vmwrite(x86::vmx::field_t::host_idtr_base, idtr.base_address));

    return {};
}

status_t vmxon_for_vcpu(vcpu_t& cpu) noexcept {
    CHECK_ASSERT(x86::vmx::prepare_for_vmxon(), "prepare_for_vmxon failed");
    CHECK_ASSERT(x86::vmx::initialize_vmstruct(cpu.vmxon_region), "initialize_vmstruct failed");

    auto vmxon_region = environment::to_physical(&cpu.vmxon_region);
    CHECK_VMX(x86::vmx::vmxon(vmxon_region));

    return {};
}

status_t setup_vmcs(context_t& context, vcpu_t& cpu) noexcept {
    CHECK_ASSERT(x86::vmx::initialize_vmstruct(cpu.vmcs), "initialize_vmstruct failed");

    CHECK_VMX(x86::vmx::vmwrite(x86::vmx::field_t::guest_link_pointer, ~0ull));

    {
        x86::vmx::ept_pointer_t eptp{};
        eptp.bits.mem_type = 3; // TODO: MAKE TYPE FOR MEMTYPE
        eptp.bits.walk_length = 3; // todo: WHAT IS WALK LENGTH
        eptp.address(environment::to_physical(&context.ept.m_pml4));

        CHECK_VMX(x86::vmx::vmwrite(x86::vmx::field_t::ctrl_ept_pointer, eptp.raw));
        CHECK_VMX(x86::vmx::vmwrite(x86::vmx::field_t::ctrl_virtual_processor_identifier, 0));
    }

    CHECK_VMX(x86::vmx::vmwrite(x86::vmx::field_t::ctrl_msr_bitmap_address, environment::to_physical(cpu.msr_bitmap)));

    CHECK(setup_vm_controls(context.wanted_vm_controls.pinbased));
    CHECK(setup_vm_controls(context.wanted_vm_controls.procbased));
    CHECK(setup_vm_controls(context.wanted_vm_controls.secondary_procbased));
    CHECK(setup_vm_controls(context.wanted_vm_controls.vmexit));
    CHECK(setup_vm_controls(context.wanted_vm_controls.vmentry));

    CHECK(setup_cr_dr_vmcs());
    CHECK(setup_segments_vmcs());

    // todo: (host and guest) rsp, rip, rflags
    // TODO: do idt

    return {};
}

}
