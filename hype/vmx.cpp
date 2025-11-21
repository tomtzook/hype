
#include <x86/vmx/segments.h>
#include <x86/vmx/controls.h>
#include <x86/interrupts.h>
#include <x86/rflags.h>

#include "environment.h"
#include "vmexit.h"
#include "vmentry.h"
#include "vmx.h"


namespace hype {

static uintn_t get_cr0_host_mask() {
    x86::cr0_t cr0(0);
    cr0.bits.paging_enable = true;

    return cr0.raw;
}

static uintn_t get_cr4_read_shadow(const uintn_t cr4_raw) {
    x86::cr4_t cr4(cr4_raw);
    cr4.bits.vmx_enable = false;

    return cr4.raw;
}

static uintn_t get_cr4_host_mask() {
    const x86::cr4_t cr4(0);

    return cr4.raw;
}

static x86::segments::selector_t host_selector(const uint16_t index) {
    x86::segments::selector_t selector{};
    selector.bits.table = x86::segments::table_type_t::gdt;
    selector.bits.rpl = 0;
    selector.bits.index = index;
    return selector;
}

template<typename _controls>
static framework::result<> setup_vm_controls(const _controls& controls) {
    auto adjusted = x86::vmx::adjust_vm_controls(controls);
    verify_vmx(x86::vmx::vmwrite(_controls::vmcs_field, adjusted.raw));

    return {};
}

template<typename _vmcs_segment_defs>
static framework::result<> setup_segment(x86::segments::table_t& table) {
    auto segment = x86::read<typename _vmcs_segment_defs::segment>();
    if (segment.bits.index == 0) {
        x86::vmx::segment_access_rights_t ar{};
        ar.bits.unusable = true;

        verify_vmx(x86::vmx::vmwrite(_vmcs_segment_defs::guest_selector, 0));
        verify_vmx(x86::vmx::vmwrite(_vmcs_segment_defs::guest_base, 0));
        verify_vmx(x86::vmx::vmwrite(_vmcs_segment_defs::guest_limit, 0));
        verify_vmx(x86::vmx::vmwrite(_vmcs_segment_defs::guest_ar, ar.raw));

        return {};
    }

    auto descriptor = table[segment.bits.index];

    verify_vmx(x86::vmx::vmwrite(_vmcs_segment_defs::guest_selector, segment.value));
    verify_vmx(x86::vmx::vmwrite(_vmcs_segment_defs::guest_base, descriptor.base_address()));
    verify_vmx(x86::vmx::vmwrite(_vmcs_segment_defs::guest_limit, descriptor.limit()));
    verify_vmx(x86::vmx::vmwrite(_vmcs_segment_defs::guest_ar, x86::vmx::segment_access_rights(descriptor).raw));

    return {};
}

static framework::result<> setup_cr_dr_vmcs(context_t& context) {
    auto cr0 = x86::read<x86::cr0_t>();
    verify_vmx(x86::vmx::vmwrite(x86::vmx::field_t::host_cr0, cr0.raw));

    x86::vmx::adjust_cr0_fixed_bits(cr0, true);
    verify_vmx(x86::vmx::vmwrite(x86::vmx::field_t::guest_cr0, cr0.raw));
    verify_vmx(x86::vmx::vmwrite(x86::vmx::field_t::ctrl_cr0_read_shadow, cr0.raw));
    verify_vmx(x86::vmx::vmwrite(x86::vmx::field_t::ctrl_cr0_guest_host_mask, get_cr0_host_mask()));

    auto cr4 = x86::read<x86::cr4_t>();
    verify_vmx(x86::vmx::vmwrite(x86::vmx::field_t::host_cr4, cr4.raw));

    x86::vmx::adjust_cr4_fixed_bits(cr4);
    verify_vmx(x86::vmx::vmwrite(x86::vmx::field_t::guest_cr4, cr4.raw));
    verify_vmx(x86::vmx::vmwrite(x86::vmx::field_t::ctrl_cr4_read_shadow, get_cr4_read_shadow(cr4.raw)));
    verify_vmx(x86::vmx::vmwrite(x86::vmx::field_t::ctrl_cr4_guest_host_mask, get_cr4_host_mask()));

    auto cr3 = x86::read<x86::cr3_t>();
    verify_vmx(x86::vmx::vmwrite(x86::vmx::field_t::guest_cr3, cr3.raw));
    cr3.raw = 0;
    cr3.ia32e.address = framework::environment::to_physical(context.page_table.m_pml4) >> x86::paging::page_bits_4k;
    verify_vmx(x86::vmx::vmwrite(x86::vmx::field_t::host_cr3, cr3.raw));

    auto efer = x86::read<x86::msr::ia32_efer_t>();
    verify_vmx(x86::vmx::vmwrite(x86::vmx::field_t::host_efer, efer.raw));
    verify_vmx(x86::vmx::vmwrite(x86::vmx::field_t::guest_efer, efer.raw));

    auto dr7 = x86::read<x86::dr7_t>();
    verify_vmx(x86::vmx::vmwrite(x86::vmx::field_t::guest_dr7, dr7.raw));

    auto debugctl = x86::read<x86::msr::ia32_debugctl_t>();
    verify_vmx(x86::vmx::vmwrite(x86::vmx::field_t::guest_debugctl, debugctl.raw));

    return {};
}

static framework::result<> setup_segments_vmcs(context_t& context) {
    auto gdtr = x86::read<x86::segments::gdtr_t>();
    auto gdt_table = x86::segments::table_t(gdtr);

    verify(setup_segment<x86::vmx::vmcs_cs_segment>(gdt_table));
    verify(setup_segment<x86::vmx::vmcs_ds_segment>(gdt_table));
    verify(setup_segment<x86::vmx::vmcs_gs_segment>(gdt_table));
    verify(setup_segment<x86::vmx::vmcs_ss_segment>(gdt_table));
    verify(setup_segment<x86::vmx::vmcs_es_segment>(gdt_table));
    verify(setup_segment<x86::vmx::vmcs_fs_segment>(gdt_table));
    verify(setup_segment<x86::vmx::vmcs_tr_segment>(gdt_table));
    verify(setup_segment<x86::vmx::vmcs_ldtr_segment>(gdt_table));

    const auto host_code_selector = host_selector(memory::gdt_t::code_descriptor_index).value;
    const auto host_data_selector = host_selector(memory::gdt_t::data_descriptor_index).value;
    const auto host_tr_selector = host_selector(memory::gdt_t::tss_descriptor_index).value;

    // load host selectors
    verify_vmx(x86::vmx::vmwrite(x86::vmx::field_t::host_ds_selector, host_data_selector));
    verify_vmx(x86::vmx::vmwrite(x86::vmx::field_t::host_ss_selector, host_data_selector));
    verify_vmx(x86::vmx::vmwrite(x86::vmx::field_t::host_es_selector, host_data_selector));
    verify_vmx(x86::vmx::vmwrite(x86::vmx::field_t::host_gs_selector, host_data_selector));
    verify_vmx(x86::vmx::vmwrite(x86::vmx::field_t::host_fs_selector, host_data_selector));
    verify_vmx(x86::vmx::vmwrite(x86::vmx::field_t::host_cs_selector, host_code_selector));
    verify_vmx(x86::vmx::vmwrite(x86::vmx::field_t::host_tr_selector, host_tr_selector));

    verify_vmx(x86::vmx::vmwrite(x86::vmx::field_t::host_tr_base, context.gdt.tr.base_address()));
    verify_vmx(x86::vmx::vmwrite(x86::vmx::field_t::host_fs_base, x86::read<x86::msr::ia32_fs_base_t>().raw));
    verify_vmx(x86::vmx::vmwrite(x86::vmx::field_t::host_gs_base, x86::read<x86::msr::ia32_gs_base_t>().raw));
    verify_vmx(x86::vmx::vmwrite(x86::vmx::field_t::host_gdtr_base, context.gdtr.base_address));

    verify_vmx(x86::vmx::vmwrite(x86::vmx::field_t::guest_gdtr_base, gdtr.base_address));
    verify_vmx(x86::vmx::vmwrite(x86::vmx::field_t::guest_gdtr_limit, gdtr.limit));

    auto idtr = x86::read<x86::interrupts::idtr_t>();
    verify_vmx(x86::vmx::vmwrite(x86::vmx::field_t::guest_idtr_base, idtr.base_address));
    verify_vmx(x86::vmx::vmwrite(x86::vmx::field_t::guest_idtr_limit, idtr.limit));
    verify_vmx(x86::vmx::vmwrite(x86::vmx::field_t::host_idtr_base, context.idtr.base_address));

    return {};
}

static framework::result<> setup_entry_exit(vcpu_t& cpu) {
    const auto host_stack_start = reinterpret_cast<uint64_t>(cpu.host_stack) + vcpu_t::stack_size;
    assert(host_stack_start % 16 == 0, "stack must be aligned to 16");

    verify_vmx(x86::vmx::vmwrite(x86::vmx::field_t::host_rsp, host_stack_start));
    verify_vmx(x86::vmx::vmwrite(x86::vmx::field_t::host_rip, reinterpret_cast<uint64_t>(asm_vm_exit)));

    verify_vmx(x86::vmx::vmwrite(x86::vmx::field_t::guest_rflags, x86::read<x86::rflags_t>().raw));
    verify_vmx(x86::vmx::vmwrite(x86::vmx::field_t::guest_rsp, host_stack_start));
    verify_vmx(x86::vmx::vmwrite(x86::vmx::field_t::guest_rip, reinterpret_cast<uint64_t>(asm_vm_entry)));

    return {};
}

framework::result<> vmxon_for_vcpu(vcpu_t& cpu) {
    assert(x86::vmx::prepare_for_vmxon(true), "prepare_for_vmxon failed");
    assert(x86::vmx::initialize_vmstruct(cpu.vmxon_region), "initialize_vmstruct failed");

    trace_debug("Doing vmxon");
    const auto vmxon_region = framework::environment::to_physical(&cpu.vmxon_region);
    verify_vmx(x86::vmx::vmxon(vmxon_region));

    return {};
}

framework::result<> setup_vmcs(context_t& context, vcpu_t& cpu) {
    // [SDM 3 24.4]
    // [SDM 3 24.5]

    verify_vmx(x86::vmx::vmwrite(x86::vmx::field_t::guest_link_pointer, ~0ull));

    {
        x86::vmx::ept_pointer_t eptp{};
        eptp.bits.mem_type = x86::mtrr::memory_type_t::writeback;
        eptp.bits.walk_length = 3;
        eptp.address(framework::environment::to_physical(&context.ept.m_pml4));

        verify_vmx(
            x86::vmx::vmwrite(x86::vmx::field_t::ctrl_ept_pointer, eptp.raw));
        verify_vmx(
            x86::vmx::vmwrite(x86::vmx::field_t::ctrl_virtual_processor_identifier, 0));
    }

    verify_vmx(x86::vmx::vmwrite(
        x86::vmx::field_t::ctrl_msr_bitmap_address,
        framework::environment::to_physical(cpu.msr_bitmap)));

    verify(setup_vm_controls(context.wanted_vm_controls.pinbased));
    verify(setup_vm_controls(context.wanted_vm_controls.procbased));
    verify(setup_vm_controls(context.wanted_vm_controls.secondary_procbased));
    verify(setup_vm_controls(context.wanted_vm_controls.vmexit));
    verify(setup_vm_controls(context.wanted_vm_controls.vmentry));

    verify(setup_cr_dr_vmcs(context));
    verify(setup_segments_vmcs(context));
    verify(setup_entry_exit(cpu));

    return {};
}

}
