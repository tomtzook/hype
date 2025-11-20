
#include <x86/vmx/vmcs.h>
#include <x86/vmx/controls.h>
#include <x86/paging/paging.h>
#include <x86/vmx/ept.h>
#include <x86/msr.h>
#include <x86/cr.h>
#include <x86/vmx/vmx.h>
#include <x86/cpuid.h>
#include <x86/rflags.h>
#include <x86/segments.h>
#include <x86/vmx/segments.h>

#include "base.h"
#include "status.h"

namespace hype {

// P1101

template<typename _control>
static framework::result<> read_controls(_control& controls) {
    uint64_t controls_raw;
    verify_vmx(x86::vmx::vmread(_control::vmcs_field, controls_raw));
    controls.raw = controls_raw;

    return {};
}

static framework::result<> verify_vmentry_exec_control_fields() {
    x86::vmx::pin_based_exec_controls_t pinbased_controls{};
    verify(read_controls(pinbased_controls));
    assert(x86::vmx::are_controls_valid(pinbased_controls), "Set PINBASED controls not supported");

    x86::vmx::processor_based_exec_controls_t cpu_exec_controls{};
    verify(read_controls(cpu_exec_controls));
    assert(x86::vmx::are_controls_valid(cpu_exec_controls), "Set EXEC controls not supported");

    const auto secondary_controls_enabled = cpu_exec_controls.bits.activate_secondary_controls;

    x86::vmx::secondary_processor_based_exec_controls_t secondary_cpu_exec_controls{};
    if (secondary_controls_enabled) {
        verify(read_controls(secondary_cpu_exec_controls));
        assert(x86::vmx::are_controls_valid(cpu_exec_controls), "Set SECONDARY EXEC controls not supported");
    }

    x86::vmx::vmexit_controls_t vmexit_controls{};
    verify(read_controls(vmexit_controls));

    if (cpu_exec_controls.bits.use_io_bitmaps) {
        uint64_t bitmap_addr;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::ctrl_io_bitmap_a_address, bitmap_addr));
        assert(x86::paging::is_page_aligned(bitmap_addr), "IO bitmap A address not aligned to page");
        assert(x86::paging::is_in_physical_address_width(bitmap_addr), "IO bitmap A address exceeds physical address width");

        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::ctrl_io_bitmap_b_address, bitmap_addr));
        assert(x86::paging::is_page_aligned(bitmap_addr), "IO bitmap B address not aligned to page");
        assert(x86::paging::is_in_physical_address_width(bitmap_addr), "IO bitmap B address exceeds physical address width");
    }

    if (cpu_exec_controls.bits.use_msr_bitmaps) {
        uint64_t bitmap_addr;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::ctrl_msr_bitmap_address, bitmap_addr));
        assert(x86::paging::is_page_aligned(bitmap_addr), "MSR bitmap address not aligned to page");
        assert(x86::paging::is_in_physical_address_width(bitmap_addr), "MSR bitmap address exceeds physical address width");
    }

    if (!pinbased_controls.bits.nmi_exiting) {
        assert(pinbased_controls.bits.virtual_nmis == 0, "Virtual NMI cannot be used when NMI Exiting is disabled");
    }

    if (!pinbased_controls.bits.virtual_nmis) {
        assert(cpu_exec_controls.bits.nmi_window_exiting == 0, "NMI Window Exiting cannot be used when Virtual NMIs are disabled");
    }

    if (secondary_controls_enabled && secondary_cpu_exec_controls.bits.virtualize_apic_accesses) {
        uint64_t addr;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::ctrl_apic_access_address, addr));
        assert(x86::paging::is_page_aligned(addr), "APIC access address not aligned to page");
        assert(x86::paging::is_in_physical_address_width(addr), "APIC access address exceeds physical address width");
    }

    if (cpu_exec_controls.bits.use_tpr_shadow) {
        uint64_t addr;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::ctrl_virtual_apic_address, addr));
        assert(x86::paging::is_page_aligned(addr), "Virtual APIC address not aligned to page");
        assert(x86::paging::is_in_physical_address_width(addr), "Virtual APIC address exceeds physical address width");

        if (secondary_controls_enabled && !secondary_cpu_exec_controls.bits.virtual_interrupt_delivery) {
            uint64_t addr2;
            verify_vmx(x86::vmx::vmread(x86::vmx::field_t::ctrl_tpr_threshold, addr2));
            assert((addr2 & 0xfffffff0) == 0, "TPR Threshold must have bits 31:4 set to 0 because Virtual Interrupt Delivery is disabled");
        }
    } else {
        assert(!secondary_controls_enabled || secondary_cpu_exec_controls.bits.virtualize_x2apic == 0, "Virtualize x2APIC cannot be used when TPR Shadow is disabled");
        assert(!secondary_controls_enabled || secondary_cpu_exec_controls.bits.apic_register_virtualization == 0, "APIC Register Virtualization cannot be used when TPR Shadow is disabled");
        assert(!secondary_controls_enabled || secondary_cpu_exec_controls.bits.virtual_interrupt_delivery == 0, "Virtual Interrupt Delivery cannot be used when TPR Shadow is disabled");
    }

    if (secondary_controls_enabled && secondary_cpu_exec_controls.bits.virtualize_x2apic) {
        assert(secondary_cpu_exec_controls.bits.virtualize_apic_accesses == 0, "Virtualize APIC Access cannot be used with Virtualize x2APIC");
    }

    if (secondary_controls_enabled && secondary_cpu_exec_controls.bits.virtual_interrupt_delivery) {
        assert(pinbased_controls.bits.external_interrupt_exiting, "External Interrupt Existing must be enabled when Virtual Interrupt Delivery is Enabled");
    }

    if (pinbased_controls.bits.process_posted_interrupts) {
        assert(secondary_controls_enabled && secondary_cpu_exec_controls.bits.virtual_interrupt_delivery, "Process Posted Interrupts is Enabled but Virtual Interrupt Delivery is not");
        assert(vmexit_controls.bits.acknowledge_interrupt_on_exit, "Process Posted Interrupts is Enabled but Acknowledge Interrupt on Exit is not");

        uint64_t interrupt_vector;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::ctrl_posted_interrupt_notification_vector, interrupt_vector));
        assert((interrupt_vector & ~0xff) == 0, "Posted Interrupt Notification Vector must be between 0 and 255 because Process Posted Interrupts is Enabled");

        uint64_t interrupt_descriptor;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::ctrl_posted_interrupt_descriptor_address, interrupt_descriptor));
        assert((interrupt_descriptor & 0x3F) == 0, "Posted Interrupt Descriptor must have bits 5:0 cleared because Process Posted Interrupts is Enabled");
        assert(x86::paging::is_in_physical_address_width(interrupt_descriptor), "Posted Interrupt Descriptor exceeds physical address width");
    }

    if (secondary_controls_enabled && secondary_cpu_exec_controls.bits.enable_vpid) {
        uint64_t vpid;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::ctrl_virtual_processor_identifier, vpid));
        assert(vpid != 0, "VPID must not be zero if Enable VPID is Requested");
    }

    if (secondary_controls_enabled && secondary_cpu_exec_controls.bits.enable_ept) {
        uint64_t eptp_raw;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::ctrl_ept_pointer, eptp_raw));
        x86::vmx::ept_pointer_t eptp{};
        eptp.raw = eptp_raw;

        const auto ept_cap = x86::read<x86::msr::ia32_vmx_ept_vpid_cap_t>();
        // eptp.bits.mem_type
        // ept_cap.bits.memory_type_write_back
        // 1102

        assert(!(eptp.bits.mem_type == x86::mtrr::memory_type_t::uncacheable && !ept_cap.bits.memory_type_uncachable), "EPTP Requested MemType Uncacheble, but it is not supported");
        assert(!(eptp.bits.mem_type == x86::mtrr::memory_type_t::writeback && !ept_cap.bits.memory_type_write_back), "EPTP Requested MemType Writeback, but it is not supported");
        assert(eptp.bits.walk_length == 3, "EPTP Walk Length must be 3");
        assert(!(eptp.bits.access_dirty_enable && !ept_cap.bits.ept_accessed_dirty), "EPTP does not support Accessed/Dirty flags");
        assert(eptp.bits.reserved0 == 0, "EPTP reserved bits 11:7 must be zero");
        assert(x86::paging::is_in_physical_address_width(eptp.bits.address), "EPTP address not within physical address width");
    }

    if (secondary_controls_enabled && secondary_cpu_exec_controls.bits.enable_pml) {
        assert(secondary_cpu_exec_controls.bits.enable_ept, "EPT must be enabled with Enable PML is on");

        uint64_t pml;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::ctrl_pml_address, pml));
        assert(x86::paging::is_page_aligned(pml), "PML address not aligned to page");
        assert(x86::paging::is_in_physical_address_width(pml), "PML address exceeds physical address width");
    }

    if (secondary_controls_enabled && (secondary_cpu_exec_controls.bits.unrestricted_guest || secondary_cpu_exec_controls.bits.mode_based_execution_control)) {
        assert(secondary_cpu_exec_controls.bits.enable_ept, "EPT must be enabled because either Unrestricted Guest or MBEC are enabled");
    }

    if (secondary_controls_enabled && secondary_cpu_exec_controls.bits.enable_vm_functions) {
        const auto allowed = x86::read<x86::msr::ia32_vmx_vmfunc_t>().bits.allowed;
        // msr[x] == 1 -> control[x] can be 1
        // msr[x] == 0 -> control[x] must be 0

        uint64_t vmfunc_control_raw;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::ctrl_vmfunc_controls, vmfunc_control_raw));
        x86::vmx::vmfunc_control_t vmfunc_control{};
        vmfunc_control.raw = vmfunc_control_raw;

        assert((vmfunc_control_raw & ~allowed) == 0, "VMFUNC Control uses unsupported bits");

        if (vmfunc_control.bits.eptp_switching) {
            assert(secondary_cpu_exec_controls.bits.enable_ept, "EPT must be enabled for VMFUNC Eptp Switching");

            uint64_t eptp_list;
            verify_vmx(x86::vmx::vmread(x86::vmx::field_t::ctrl_ept_pointer_list_address, eptp_list));
            assert(x86::paging::is_page_aligned(eptp_list), "EPTP List address not aligned to page");
            assert(x86::paging::is_in_physical_address_width(eptp_list), "EPTP List address exceeds physical address width");
        }
    }

    if (secondary_controls_enabled && secondary_cpu_exec_controls.bits.vmcs_shadowing) {
        uint64_t bitmap_addr;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::ctrl_vmread_bitmap_address, bitmap_addr));
        assert(x86::paging::is_page_aligned(bitmap_addr), "VMREAD Bitmap address not aligned to page");
        assert(x86::paging::is_in_physical_address_width(bitmap_addr), "VMREAD Bitmap address exceeds physical address width");

        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::ctrl_vmwrite_bitmap_address, bitmap_addr));
        assert(x86::paging::is_page_aligned(bitmap_addr), "VMWRITE Bitmap address not aligned to page");
        assert(x86::paging::is_in_physical_address_width(bitmap_addr), "VMWRITE Bitmap address exceeds physical address width");
    }

    if (secondary_controls_enabled && secondary_cpu_exec_controls.bits.ept_violation_ve) {
        uint64_t exc_addr;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::ctrl_virtualization_exception_information_address, exc_addr));
        assert(x86::paging::is_page_aligned(exc_addr), "Virtualization Exception Info address not aligned to page");
        assert(x86::paging::is_in_physical_address_width(exc_addr), "Virtualization Exception Info address exceeds physical address width");
    }

    return {};
}

static framework::result<> verify_msr_store_load(const uint64_t count, const uint64_t address) {
    if (count == 0) {
        return {};
    }

    assert((address & 0xf) == 0, "MSR Store/Load Address bits 4:0 must be 0");
    assert(x86::paging::is_in_physical_address_width(address), "MSR Store/Load Address address exceeds physical address width");

    const auto last_addr = address + (count * 16) - 1;
    assert(x86::paging::is_in_physical_address_width(last_addr), "MSR Store/Load last byte address exceeds physical address width");

    const auto vmx_basic = x86::read<x86::msr::ia32_vmx_basic_t>();
    if (vmx_basic.bits.physaddr_width_type) {
        assert((address & 0xffffffff00000000) == 0, "MSR Store/Load Address bits 63:32 must be 0");
        assert((last_addr & 0xffffffff00000000) == 0, "MSR Store/Load last byte bits 63:32 must be 0");
    }

    return {};
}

static framework::result<> verify_vmentry_exit_control_fields() {
    x86::vmx::vmexit_controls_t vmexit_controls{};
    verify(read_controls(vmexit_controls));
    assert(x86::vmx::are_controls_valid(vmexit_controls), "Set VMEXIT controls not supported");

    x86::vmx::pin_based_exec_controls_t pinbased_controls{};
    verify(read_controls(pinbased_controls));

    if (!pinbased_controls.bits.activate_preemption_timer) {
        assert(vmexit_controls.bits.save_preemption_timer_value == 0, "Save Preemption Timer value must be 0 because Preemption Timer not activated");
    }

    {
        uint64_t store_count;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::ctrl_vmexit_msr_store_count, store_count));
        uint64_t store_address;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::ctrl_vmexit_msr_store_address, store_address));

        verify(verify_msr_store_load(store_count, store_address));
    }

    {
        uint64_t load_count;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::ctrl_vmexit_msr_load_count, load_count));
        uint64_t load_address;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::ctrl_vmexit_msr_load_address, load_address));

        verify(verify_msr_store_load(load_count, load_address));
    }

    return {};
}

static framework::result<> verify_vmentry_entry_control_fields() {
    x86::vmx::vmentery_controls_t vmentry_controls{};
    verify(read_controls(vmentry_controls));
    assert(x86::vmx::are_controls_valid(vmentry_controls), "Set VMENTRY controls not supported");

    x86::vmx::pin_based_exec_controls_t pinbased_controls{};
    verify(read_controls(pinbased_controls));
    x86::vmx::processor_based_exec_controls_t cpu_exec_controls{};
    verify(read_controls(cpu_exec_controls));

    const auto secondary_controls_enabled = cpu_exec_controls.bits.activate_secondary_controls;
    x86::vmx::secondary_processor_based_exec_controls_t secondary_cpu_exec_controls{};
    if (secondary_controls_enabled) {
        verify(read_controls(secondary_cpu_exec_controls));
    }

    // P1104
    x86::vmx::vmentry_interruption_info_t vmentry_interruption_info{};
    {
        uint64_t raw;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::ctrl_vmentry_interruption_information_field, raw));
        vmentry_interruption_info.raw = raw;
    }
    if (vmentry_interruption_info.bits.valid) {
        assert(vmentry_interruption_info.bits.type != x86::vmx::vmentry_interrupt_type_t::reserved, "VMENTRY Interrupt Info Type is set to Reserved");
        assert(!(!cpu_exec_controls.bits.monitor_trap_flag && vmentry_interruption_info.bits.type == x86::vmx::vmentry_interrupt_type_t::other_event), "VMENTRY Interrupt Info Type is set to OtherEvent but not supported");
        assert(vmentry_interruption_info.bits.type != x86::vmx::vmentry_interrupt_type_t::nmi || vmentry_interruption_info.bits.vector == 2, "VMENTRY Interrupt Info Type is NMI and Vector is not 2");
        assert(vmentry_interruption_info.bits.type != x86::vmx::vmentry_interrupt_type_t::hardware_exception || vmentry_interruption_info.bits.vector <= 31, "VMENTRY Interrupt Info Type is HARDWARE EXCEPTION and Vector is larger than 31");
        assert(vmentry_interruption_info.bits.type != x86::vmx::vmentry_interrupt_type_t::other_event || vmentry_interruption_info.bits.vector == 0, "VMENTRY Interrupt Info Type is OTHER EVENT and Vector is not 0");

        if (vmentry_interruption_info.bits.deliver_error_code == 1) {
            uint64_t raw;
            verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_cr0, raw));
            x86::cr0_t guest_cr0(raw);

            assert(
                ((!secondary_controls_enabled || secondary_cpu_exec_controls.bits.unrestricted_guest == 0) || guest_cr0.bits.protection_enable) &&
                vmentry_interruption_info.bits.type == x86::vmx::vmentry_interrupt_type_t::hardware_exception &&
                is_any_of(vmentry_interruption_info.bits.vector, 8, 10, 11, 12, 13, 14, 17),
                "VMENTRY Interrupt Info Deliver Error code is set, but not allowed"
            );

            uint64_t error_code;
            verify_vmx(x86::vmx::vmread(x86::vmx::field_t::ctrl_vmentry_exception_error_code, error_code));
            assert((error_code & 0xffbf0000) == 0, "VMENTRY Exception Error Code bits 31:15 must be 0");
        }// P1104

        assert(vmentry_interruption_info.bits.reserved == 0, "VMENTRY Interrupt Info reserved must be 0");

        if (is_any_of(vmentry_interruption_info.bits.type,
            x86::vmx::vmentry_interrupt_type_t::software_interrupt,
            x86::vmx::vmentry_interrupt_type_t::software_exception,
            x86::vmx::vmentry_interrupt_type_t::privileged_software_exception)) {
            uint64_t length;
            verify_vmx(x86::vmx::vmread(x86::vmx::field_t::ctrl_vmentry_instruction_length, length));

            auto vmx_misc = x86::read<x86::msr::ia32_vmx_misc_t>();

            assert((length > 0 && length <= 15) || (vmx_misc.bits.software_inject && length == 0), "VMENTRY Instruction Length invalid value");
        }
    }

    {
        uint64_t load_count;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::ctrl_vmentry_msr_load_count, load_count));
        uint64_t load_address;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::ctrl_vmentry_msr_load_address, load_address));

        verify(verify_msr_store_load(load_count, load_address));
    }

    // TODO: HOW TO CHECK IF GUEST IN SMM MODE
    /*bool cpu_in_smm = false; // todo: how
    if (!cpu_in_smm) {
        assert(vmentry_controls.bits.entry_to_smm == 0, "Entry to SMM must be off when not in SMM");
        assert(vmentry_controls.bits.deactivate_dual_monitor == 0, "Deactivate Dual Monitor must be off when not in SMM");
    }*/

    assert((vmentry_controls.bits.entry_to_smm & vmentry_controls.bits.deactivate_dual_monitor) == 0, "Entry to SMM and Deactivate Dual Monitor cannot both be 1");

    return {};
}

static framework::result<> verify_host_state() {
    x86::vmx::processor_based_exec_controls_t cpu_exec_controls{};
    verify(read_controls(cpu_exec_controls));
    x86::vmx::vmentery_controls_t vmentry_controls{};
    verify(read_controls(vmentry_controls));
    x86::vmx::vmexit_controls_t vmexit_controls{};
    verify(read_controls(vmexit_controls));

    uint64_t cr0;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::host_cr0, cr0));
    uint64_t cr3;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::host_cr3, cr3));
    uint64_t cr4;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::host_cr4, cr4));

    assert(x86::vmx::is_cr0_valid(cr0), "Host CR0 not valid");
    assert(x86::vmx::is_cr4_valid(cr4), "Host CR4 not valid");

    const auto features = x86::cpuid<x86::cpuid_extended_processor_info_t>();
    if (features.edx.bits.long_mode) {
        assert((cr3 & 0xffd0000000000000) == 0, "Host CR3 bits 63:52 must be 0");
        assert(x86::paging::is_in_physical_address_width(cr3), "Host CR3 bits 51:12 (max physical address width) must be 0");
    }

    {
        uint64_t sysenter_esp;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::host_sysenter_esp, sysenter_esp));
        uint64_t sysenter_eip;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::host_sysenter_eip, sysenter_eip));

        assert(x86::paging::is_canonical(sysenter_esp), "Host SYSENTER ESP not cannoical");
        assert(x86::paging::is_canonical(sysenter_eip), "Host SYSENTER EIP not cannoical");
    }

    if (vmexit_controls.bits.load_perf_glob) {
        uint64_t raw;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::host_perf_global_ctrl, raw));
        x86::msr::ia32_pref_global_ctrl_t msr(raw);

        assert(msr.bits.reserved0 == 0 && msr.bits.reserved1 == 0, "Host Perf Global reserved bits must be 0");
    }

    if (vmexit_controls.bits.load_pat) {
        uint64_t raw;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::host_pat, raw));
        x86::msr::ia32_pat_t msr(raw);

        assert(is_any_of(msr.bits.pa0, 0, 1, 4, 5, 6, 7), "Host IA32_PAT p0 has invalid type");
        assert(is_any_of(msr.bits.pa1, 0, 1, 4, 5, 6, 7), "Host IA32_PAT p1 has invalid type");
        assert(is_any_of(msr.bits.pa2, 0, 1, 4, 5, 6, 7), "Host IA32_PAT p2 has invalid type");
        assert(is_any_of(msr.bits.pa3, 0, 1, 4, 5, 6, 7), "Host IA32_PAT p3 has invalid type");
        assert(is_any_of(msr.bits.pa4, 0, 1, 4, 5, 6, 7), "Host IA32_PAT p4 has invalid type");
        assert(is_any_of(msr.bits.pa5, 0, 1, 4, 5, 6, 7), "Host IA32_PAT p5 has invalid type");
        assert(is_any_of(msr.bits.pa6, 0, 1, 4, 5, 6, 7), "Host IA32_PAT p6 has invalid type");
        assert(is_any_of(msr.bits.pa7, 0, 1, 4, 5, 6, 7), "Host IA32_PAT p7 has invalid type");
    }

    if (vmexit_controls.bits.load_efer) {
        uint64_t raw;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::host_efer, raw));
        x86::msr::ia32_efer_t msr(raw);

        assert(msr.bits.reserved0 == 0 && msr.bits.reserved1 == 0 && msr.bits.reserved2, "Host EFER reserved bits must be 0");
        assert(msr.bits.lme == vmexit_controls.bits.host_address_space_size, "Host EFER lme must be according to Host Address Space Size");
        assert(msr.bits.lma == vmexit_controls.bits.host_address_space_size, "Host EFER lma must be according to Host Address Space Size");
    }

    {
        uint64_t cs;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::host_cs_selector, cs));
        uint64_t ds;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::host_ds_selector, ds));
        uint64_t ss;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::host_ss_selector, ss));
        uint64_t es;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::host_es_selector, es));
        uint64_t fs;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::host_fs_selector, fs));
        uint64_t gs;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::host_gs_selector, gs));
        uint64_t tr;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::host_tr_selector, tr));

        assert((cs & 0x7) == 0, "Host CS Selector RPL and TI must be 0");
        assert((ds & 0x7) == 0, "Host DS Selector RPL and TI must be 0");
        assert((ss & 0x7) == 0, "Host SS Selector RPL and TI must be 0");
        assert((es & 0x7) == 0, "Host ES Selector RPL and TI must be 0");
        assert((fs & 0x7) == 0, "Host FS Selector RPL and TI must be 0");
        assert((gs & 0x7) == 0, "Host GS Selector RPL and TI must be 0");
        assert((tr & 0x7) == 0, "Host TR Selector RPL and TI must be 0");
        assert(cs != 0, "Host CS Selector cannot be 0");
        assert(tr != 0, "Host TR Selector cannot be 0");
        assert(vmexit_controls.bits.host_address_space_size || ss != 0, "Host SS Selector cannot be 0 when Host Address Space Size is 0");

        if (features.edx.bits.long_mode) {
            uint64_t address;
            verify_vmx(x86::vmx::vmread(x86::vmx::field_t::host_fs_base, address));
            assert(x86::paging::is_canonical(address), "Host FS Base must be canonical");
            verify_vmx(x86::vmx::vmread(x86::vmx::field_t::host_gs_base, address));
            assert(x86::paging::is_canonical(address), "Host GS Base must be canonical");
            verify_vmx(x86::vmx::vmread(x86::vmx::field_t::host_tr_base, address));
            assert(x86::paging::is_canonical(address), "Host TR Base must be canonical");
            verify_vmx(x86::vmx::vmread(x86::vmx::field_t::host_gdtr_base, address));
            assert(x86::paging::is_canonical(address), "Host GDTR Base must be canonical");
            verify_vmx(x86::vmx::vmread(x86::vmx::field_t::host_idtr_base, address));
            assert(x86::paging::is_canonical(address), "Host IDTR Base must be canonical");
        }
    }

    if (features.edx.bits.long_mode) {
        const auto efer = x86::read<x86::msr::ia32_efer_t>();
        if (!efer.bits.lma) {
            assert(vmentry_controls.bits.ia32e_mode_guest == 0, "IA32 Mode Guest must be 0 since EFER.LMA=0");
            assert(vmexit_controls.bits.host_address_space_size == 0, "Host Address Space Size must be 0 since EFER.LMA=0");
        } else {
            assert(vmexit_controls.bits.host_address_space_size, "Host Address Space Size must be 1 since EFER.LMA=1");
        }

        uint64_t rip;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::host_rip, rip));

        if (!vmexit_controls.bits.host_address_space_size) {
            assert(vmentry_controls.bits.ia32e_mode_guest == 0, "IA32 Mode Guest must be 0 since Host Address Space Size is 0");
            assert(x86::cr4_t{cr4}.bits.pcid_enable == 0, "PCIDE must be 0 since Host Address Space Size is 0");
            assert((rip & 0xffffffff00000000) == 0, "Host RIP bits 63:32 must be 0 since Host Address Space Size is 0");
        } else {
            assert(x86::cr4_t{cr4}.bits.physical_address_extension == 1, "PAE must be 1 since Host Address Space Size is 1");
            assert(x86::paging::is_canonical(rip), "RIP must be canonical since Host Address Space Size is 1");
        }
    } else {
        assert(vmentry_controls.bits.ia32e_mode_guest == 0, "IA32 Mode Guest must be 0 since cpu is not 64bit");
        assert(vmexit_controls.bits.host_address_space_size == 0, "Host Address Space Size must be 0 since cpu is not 64bit");
    }

    return {};
}

static framework::result<> verify_guest_registers() {
    x86::vmx::processor_based_exec_controls_t cpu_exec_controls{};
    verify(read_controls(cpu_exec_controls));
    const auto secondary_controls_enabled = cpu_exec_controls.bits.activate_secondary_controls;
    x86::vmx::secondary_processor_based_exec_controls_t secondary_cpu_exec_controls{};
    if (secondary_controls_enabled) {
        verify(read_controls(secondary_cpu_exec_controls));
    }
    const auto is_unrestricted_guest = secondary_controls_enabled && secondary_cpu_exec_controls.bits.unrestricted_guest;

    x86::vmx::vmentery_controls_t vmentry_controls{};
    verify(read_controls(vmentry_controls));
    x86::vmx::vmexit_controls_t vmexit_controls{};
    verify(read_controls(vmexit_controls));

    uint64_t cr0;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_cr0, cr0));
    uint64_t cr3;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_cr3, cr3));
    uint64_t cr4;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_cr4, cr4));

    assert(x86::vmx::is_cr0_valid(cr0, is_unrestricted_guest), "Guest CR0 not valid");
    assert(x86::vmx::is_cr4_valid(cr4), "Guest CR4 not valid");
    assert(!x86::cr0_t{cr0}.bits.paging_enable || x86::cr0_t{cr0}.bits.protection_enable, "Guest CR0.PG on but PE is not");

    if (vmentry_controls.bits.load_debug_controls) {
        uint64_t raw;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_debugctl, raw));

        x86::msr::ia32_debugctl_t debugctl(raw);
        assert(debugctl.bits.reserved0 == 0 && debugctl.bits.reserved1 == 0 && debugctl.bits.reserved2, "Guest IA32_DEBUGCTL must have reserved bits set to 0");
    }

    const auto features = x86::cpuid<x86::cpuid_extended_processor_info_t>();
    const bool is_cpu_intel64 = features.edx.bits.long_mode;

    if (is_cpu_intel64) {
        assert(!vmentry_controls.bits.ia32e_mode_guest || (x86::cr0_t{cr0}.bits.paging_enable && x86::cr4_t{cr4}.bits.physical_address_extension), "Guest set to IA32e mode, but CR0.PG or CR4.PAE are not 1");
        assert(vmentry_controls.bits.ia32e_mode_guest || !x86::cr4_t{cr4}.bits.pcid_enable, "Guest not set to IA32e mode, but CR4.PCIDE is not 0");
        assert((cr3 & 0xffd0000000000000) == 0, "Guest CR3 bits 63:52 must be 0");
        assert(x86::paging::is_in_physical_address_width(cr3), "Guest CR3 bits 51:12 (max physical address width) must be 0");

        uint64_t dr7;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_dr7, dr7));
        assert(!vmentry_controls.bits.load_debug_controls || x86::dr7_t{dr7}.bits.reserved4 == 0, "Guest DR7 bits 63:32 not set to 0");

        uint64_t sysenter_esp;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_sysenter_esp, sysenter_esp));
        uint64_t sysenter_eip;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_sysenter_eip, sysenter_eip));
        assert(x86::paging::is_canonical(sysenter_esp), "Guest SYSENTER ESP not cannoical");
        assert(x86::paging::is_canonical(sysenter_eip), "Guest SYSENTER EIP not cannoical");
    }

    if (vmentry_controls.bits.load_perf_global_ctrl) {
        uint64_t raw;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_perf_global_ctrl, raw));
        x86::msr::ia32_pref_global_ctrl_t msr(raw);

        assert(msr.bits.reserved0 == 0 && msr.bits.reserved1 == 0, "Guest Perf Global reserved bits must be 0");
    }

    if (vmentry_controls.bits.load_pat) {
        uint64_t raw;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_pat, raw));
        x86::msr::ia32_pat_t msr(raw);

        assert(is_any_of(msr.bits.pa0, 0, 1, 4, 5, 6, 7), "Guest IA32_PAT p0 has invalid type");
        assert(is_any_of(msr.bits.pa1, 0, 1, 4, 5, 6, 7), "Guest IA32_PAT p1 has invalid type");
        assert(is_any_of(msr.bits.pa2, 0, 1, 4, 5, 6, 7), "Guest IA32_PAT p2 has invalid type");
        assert(is_any_of(msr.bits.pa3, 0, 1, 4, 5, 6, 7), "Guest IA32_PAT p3 has invalid type");
        assert(is_any_of(msr.bits.pa4, 0, 1, 4, 5, 6, 7), "Guest IA32_PAT p4 has invalid type");
        assert(is_any_of(msr.bits.pa5, 0, 1, 4, 5, 6, 7), "Guest IA32_PAT p5 has invalid type");
        assert(is_any_of(msr.bits.pa6, 0, 1, 4, 5, 6, 7), "Guest IA32_PAT p6 has invalid type");
        assert(is_any_of(msr.bits.pa7, 0, 1, 4, 5, 6, 7), "Guest IA32_PAT p7 has invalid type");
    }

    if (vmentry_controls.bits.load_efer) {
        uint64_t raw;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_efer, raw));
        x86::msr::ia32_efer_t msr(raw);

        assert(msr.bits.reserved0 == 0 && msr.bits.reserved1 == 0 && msr.bits.reserved2, "Guest EFER reserved bits must be 0");
        assert(msr.bits.lma == vmentry_controls.bits.ia32e_mode_guest, "Guest EFER.LMA must be according to Entry IA32e Mode Guest");
        assert(!x86::cr0_t{cr0}.bits.paging_enable || msr.bits.lma == msr.bits.lme, "Guest EFER.LMA should be equal to EFER.LME");
    }

    // todo: handle load ia32_bndcfg, field missing

    {
        uint64_t rip;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_rip, rip));
        uint64_t rflags_raw;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_rflags, rflags_raw));

        if (is_cpu_intel64) {
            uint64_t cs_access_rights_raw;
            verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_cs_access_rights, cs_access_rights_raw));

            const auto cs_l = x86::vmx::segment_access_rights_t{static_cast<uint32_t>(cs_access_rights_raw)}.bits.long_mode;
            assert((cs_l && vmentry_controls.bits.ia32e_mode_guest) || (rip & 0xffffffff00000000) == 0, "Guest RIP bits 63:32 must be 0 when guest is not IA32e mode");
            assert(!(cs_l && vmentry_controls.bits.ia32e_mode_guest) || rip == x86::paging::sign_extended(rip), "Guest RIP is not properly sign extended");
        }

        x86::rflags_t rflags{rflags_raw};
        assert(rflags.bits.reserved0 == 1, "Guest RFLAGS bit 1 must be 1");
        assert(rflags.bits.reserved1 == 0 && rflags.bits.reserved2 == 0 && rflags.bits.reserved3 == 0 && rflags.bits.reserved4 == 0, "Guest RFLAGS Reserved bits must be 0");
        assert((!vmentry_controls.bits.ia32e_mode_guest && x86::cr0_t{cr0}.bits.protection_enable) || rflags.bits.virtual_8086 == 0, "Guest RFLAGS.VM must be 0");

        uint64_t interrupt_info_raw;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::ctrl_vmentry_interruption_information_field, interrupt_info_raw));
        x86::vmx::vmentry_interruption_info_t interruption_info{};
        interruption_info.raw = interrupt_info_raw;
        assert(!(interruption_info.bits.valid && interruption_info.bits.type == x86::vmx::vmentry_interrupt_type_t::external_interrupt) || rflags.bits.interrupt_enable_flag, "Guest RFLAGS.IF must be 1");
    }

    return {};
}

static framework::result<> verify_guest_segments() {
    x86::vmx::processor_based_exec_controls_t cpu_exec_controls{};
    verify(read_controls(cpu_exec_controls));
    const auto secondary_controls_enabled = cpu_exec_controls.bits.activate_secondary_controls;
    x86::vmx::secondary_processor_based_exec_controls_t secondary_cpu_exec_controls{};
    if (secondary_controls_enabled) {
        verify(read_controls(secondary_cpu_exec_controls));
    }
    const auto is_unrestricted_guest = secondary_controls_enabled && secondary_cpu_exec_controls.bits.unrestricted_guest;
    const auto features = x86::cpuid<x86::cpuid_extended_processor_info_t>();
    x86::vmx::vmentery_controls_t vmentry_controls{};
    verify(read_controls(vmentry_controls));
    uint64_t rflags;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_rflags, rflags));

    const bool is_cpu_intel64 = features.edx.bits.long_mode;
    const bool is_ia32e_mode = is_cpu_intel64 && vmentry_controls.bits.ia32e_mode_guest;
    const bool is_virtual_8086 = x86::rflags_t{rflags}.bits.virtual_8086;

    uint64_t tr_selector;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_tr_selector, tr_selector));
    uint64_t tr_base;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_tr_base, tr_base));
    uint64_t tr_limit;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_tr_limit, tr_limit));
    uint64_t tr_access_rights_raw;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_tr_access_rights, tr_access_rights_raw));
    uint64_t cs_selector;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_cs_selector, cs_selector));
    uint64_t cs_base;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_cs_base, cs_base));
    uint64_t cs_limit;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_cs_limit, cs_limit));
    uint64_t cs_access_rights_raw;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_cs_access_rights, cs_access_rights_raw));
    uint64_t ds_selector;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_ds_selector, ds_selector));
    uint64_t ds_base;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_ds_base, ds_base));
    uint64_t ds_limit;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_ds_limit, ds_limit));
    uint64_t ds_access_rights_raw;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_ds_access_rights, ds_access_rights_raw));
    uint64_t ss_selector;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_ss_selector, ss_selector));
    uint64_t ss_base;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_ss_base, ss_base));
    uint64_t ss_limit;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_ss_limit, ss_limit));
    uint64_t ss_access_rights_raw;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_ss_access_rights, ss_access_rights_raw));
    uint64_t es_selector;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_es_selector, es_selector));
    uint64_t es_base;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_es_base, es_base));
    uint64_t es_limit;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_es_limit, es_limit));
    uint64_t es_access_rights_raw;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_es_access_rights, es_access_rights_raw));
    uint64_t gs_selector;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_gs_selector, gs_selector));
    uint64_t gs_base;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_gs_base, gs_base));
    uint64_t gs_limit;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_gs_limit, gs_limit));
    uint64_t gs_access_rights_raw;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_gs_access_rights, gs_access_rights_raw));
    uint64_t fs_selector;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_fs_selector, fs_selector));
    uint64_t fs_base;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_fs_base, fs_base));
    uint64_t fs_limit;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_fs_limit, fs_limit));
    uint64_t fs_access_rights_raw;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_fs_access_rights, fs_access_rights_raw));
    uint64_t ldtr_selector;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_ldtr_selector, ldtr_selector));
    uint64_t ldtr_base;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_ldtr_base, ldtr_base));
    uint64_t ldtr_limit;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_fs_limit, ldtr_limit));
    uint64_t ldtr_access_rights_raw;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_ldtr_access_rights, ldtr_access_rights_raw));

    assert(x86::segments::selector_t{static_cast<uint16_t>(tr_selector)}.bits.table == x86::segments::table_type_t::gdt, "Guest TR.TI must be 0");
    assert(
        x86::vmx::segment_access_rights_t{static_cast<uint32_t>(ldtr_access_rights_raw)}.bits.unusable ||
        x86::segments::selector_t{static_cast<uint16_t>(tr_selector)}.bits.table == x86::segments::table_type_t::gdt,
        "Guest LDTR.TI must be 0");
    if (!is_virtual_8086 && !is_unrestricted_guest) {
        assert(x86::segments::selector_t{static_cast<uint16_t>(ss_selector)}.bits.rpl ==
            x86::segments::selector_t{static_cast<uint16_t>(cs_selector)}.bits.rpl, "Guest SS.RPL must be the same as CS.RPL");
    }

    if (is_virtual_8086) {
        assert(cs_base == cs_selector << 4, "Guest CS Base must be = CS Selector << 4");
        assert(ss_base == ss_selector << 4, "Guest SS Base must be = SS Selector << 4");
        assert(ds_base == ds_selector << 4, "Guest DS Base must be = DS Selector << 4");
        assert(es_base == es_selector << 4, "Guest ES Base must be = ES Selector << 4");
        assert(fs_base == fs_selector << 4, "Guest FS Base must be = FS Selector << 4");
        assert(gs_base == gs_selector << 4, "Guest GS Base must be = GS Selector << 4");
    }

    if (is_cpu_intel64) {
        assert(x86::paging::is_canonical(tr_base), "Guest TR Base must be canonical");
        assert(x86::paging::is_canonical(fs_base), "Guest FS Base must be canonical");
        assert(x86::paging::is_canonical(gs_base), "Guest GS Base must be canonical");

        assert(x86::vmx::segment_access_rights_t{static_cast<uint32_t>(ldtr_access_rights_raw)}.bits.unusable ||
            x86::paging::is_canonical(ldtr_base), "Guest LDTR Base must be canonical");
        assert((cs_base & ~((1ull << 32) - 1)) == 0, "Guest CS Base bits 63:32 must be 0");
        assert(x86::vmx::segment_access_rights_t{static_cast<uint32_t>(ss_access_rights_raw)}.bits.unusable || (ss_base & ~((1ull << 32) - 1)) == 0, "Guest SS Base bits 63:32 must be 0");
        assert(x86::vmx::segment_access_rights_t{static_cast<uint32_t>(ds_access_rights_raw)}.bits.unusable || (ds_base & ~((1ull << 32) - 1)) == 0, "Guest DS Base bits 63:32 must be 0");
        assert(x86::vmx::segment_access_rights_t{static_cast<uint32_t>(es_access_rights_raw)}.bits.unusable || (es_base & ~((1ull << 32) - 1)) == 0, "Guest ES Base bits 63:32 must be 0");
    }

    if (is_virtual_8086) {
        assert(cs_limit == 0xffff, "Guest CS must be 0xffff");
        assert(ds_limit == 0xffff, "Guest DS must be 0xffff");
        assert(ss_limit == 0xffff, "Guest SS must be 0xffff");
        assert(es_limit == 0xffff, "Guest ES must be 0xffff");
        assert(gs_limit == 0xffff, "Guest GS must be 0xffff");
        assert(fs_limit == 0xffff, "Guest FS must be 0xffff");
    }

    if (is_virtual_8086) {
        assert(cs_access_rights_raw == 0xf3, "Guest CS Access Rights must be 0xf3");
        assert(ds_access_rights_raw == 0xf3, "Guest DS Access Rights must be 0xf3");
        assert(ss_access_rights_raw == 0xf3, "Guest SS Access Rights must be 0xf3");
        assert(es_access_rights_raw == 0xf3, "Guest ES Access Rights must be 0xf3");
        assert(gs_access_rights_raw == 0xf3, "Guest GS Access Rights must be 0xf3");
        assert(fs_access_rights_raw == 0xf3, "Guest FS Access Rights must be 0xf3");
    } else {
        const auto cs_ar = x86::vmx::segment_access_rights_t{static_cast<uint32_t>(cs_access_rights_raw)};
        const auto ds_ar = x86::vmx::segment_access_rights_t{static_cast<uint32_t>(ds_access_rights_raw)};
        const auto ss_ar = x86::vmx::segment_access_rights_t{static_cast<uint32_t>(ss_access_rights_raw)};
        const auto es_ar = x86::vmx::segment_access_rights_t{static_cast<uint32_t>(es_access_rights_raw)};
        const auto gs_ar = x86::vmx::segment_access_rights_t{static_cast<uint32_t>(gs_access_rights_raw)};
        const auto fs_ar = x86::vmx::segment_access_rights_t{static_cast<uint32_t>(fs_access_rights_raw)};
        const auto cs_ar_type = cs_ar.bits.type;
        const auto ds_ar_type = ds_ar.bits.type;
        const auto ss_ar_type = ss_ar.bits.type;
        const auto es_ar_type = es_ar.bits.type;
        const auto fs_ar_type = fs_ar.bits.type;
        const auto gs_ar_type = gs_ar.bits.type;

        if (is_unrestricted_guest) {
            assert(is_any_of(cs_ar_type, 3, 9, 11, 13, 15), "Guest CS Access Rights Type not valid");
        } else {
            assert(is_any_of(cs_ar_type, 9, 11, 13, 15), "Guest CS Access Rights Type not valid");
        }

        assert(ss_ar.bits.unusable || is_any_of(ss_ar_type, 3, 7), "Guest SS Access Rights Type not valid");
        assert(ds_ar.bits.unusable ||
            ((ds_ar_type & 1) == 1 && ((ds_ar_type & (1 << 3)) == 0 || (ds_ar_type & (1 << 1)) == 1)),
            "Guest DS Access Rights Type not valid");
        assert(es_ar.bits.unusable ||
            ((es_ar_type & 1) == 1 && ((es_ar_type & (1 << 3)) == 0 || (es_ar_type & (1 << 1)) == 1)),
            "Guest ES Access Rights Type not valid");
        assert(gs_ar.bits.unusable ||
            ((gs_ar_type & 1) == 1 && ((gs_ar_type & (1 << 3)) == 0 || (gs_ar_type & (1 << 1)) == 1)),
            "Guest GS Access Rights Type not valid");
        assert(fs_ar.bits.unusable ||
            ((fs_ar_type & 1) == 1 && ((fs_ar_type & (1 << 3)) == 0 || (fs_ar_type & (1 << 1)) == 1)),
            "Guest FS Access Rights Type not valid");

        assert(cs_ar.bits.s, "Guest CS Access Rights S must be 1");
        assert(ds_ar.bits.unusable || ds_ar.bits.s, "Guest DS Access Rights S must be 1");
        assert(ss_ar.bits.unusable || ss_ar.bits.s, "Guest SS Access Rights S must be 1");
        assert(es_ar.bits.unusable || es_ar.bits.s, "Guest ES Access Rights S must be 1");
        assert(fs_ar.bits.unusable || fs_ar.bits.s, "Guest FS Access Rights S must be 1");
        assert(gs_ar.bits.unusable || gs_ar.bits.s, "Guest GS Access Rights S must be 1");

        assert(cs_ar_type != 3 || (is_unrestricted_guest && cs_ar.bits.dpl == 0), "Guest CS Access Rights DPL must be 0");
        assert((cs_ar_type != 9 && cs_ar_type != 1) || (cs_ar.bits.dpl == ss_ar.bits.dpl), "Guest CS Access Rights DPL must be same as SS DPL");
        assert((cs_ar_type != 13 && cs_ar_type != 15) || (cs_ar.bits.dpl <= ss_ar.bits.dpl), "Guest CS Access Rights DPL must be less or equal to SS DPL");

        assert(!is_unrestricted_guest || ss_ar.bits.dpl == x86::segments::selector_t{static_cast<uint16_t>(ss_selector)}.bits.rpl, "Guest SS Access Rights DPL must same as SS Selector RPL");
        assert(!(is_unrestricted_guest && !ds_ar.bits.unusable && ds_ar_type <= 11) ||
            ds_ar.bits.dpl >= x86::segments::selector_t{static_cast<uint16_t>(ds_selector)}.bits.rpl, "Guest DS DPL cannot be less than RPL");
        assert(!(is_unrestricted_guest && !es_ar.bits.unusable && es_ar_type <= 11) ||
            es_ar.bits.dpl >= x86::segments::selector_t{static_cast<uint16_t>(es_selector)}.bits.rpl, "Guest ES DPL cannot be less than RPL");
        assert(!(is_unrestricted_guest && !fs_ar.bits.unusable && fs_ar_type <= 11) ||
            fs_ar.bits.dpl >= x86::segments::selector_t{static_cast<uint16_t>(fs_selector)}.bits.rpl, "Guest FS DPL cannot be less than RPL");
        assert(!(is_unrestricted_guest && !gs_ar.bits.unusable && gs_ar_type <= 11) ||
            gs_ar.bits.dpl >= x86::segments::selector_t{static_cast<uint16_t>(gs_selector)}.bits.rpl, "Guest GS DPL cannot be less than RPL");

        assert(cs_ar.bits.present, "Guest CS Access Rights P must be 1");
        assert(ds_ar.bits.unusable || ds_ar.bits.present, "Guest DS Access Rights P must be 1");
        assert(ss_ar.bits.unusable || ss_ar.bits.present, "Guest SS Access Rights P must be 1");
        assert(es_ar.bits.unusable || es_ar.bits.present, "Guest ES Access Rights P must be 1");
        assert(fs_ar.bits.unusable || fs_ar.bits.present, "Guest FS Access Rights P must be 1");
        assert(gs_ar.bits.unusable || gs_ar.bits.present, "Guest GS Access Rights P must be 1");

        assert(cs_ar.bits.reserved0 == 0 && cs_ar.bits.reserved1 == 0, "Guest CS Access Rights Reserved must be 0");
        assert(ds_ar.bits.unusable || (ds_ar.bits.reserved0 == 0 && ds_ar.bits.reserved1 == 0), "Guest DS Access Rights Reserved must be 0");
        assert(ss_ar.bits.unusable || (ss_ar.bits.reserved0 == 0 && ss_ar.bits.reserved1 == 0), "Guest SS Access Rights Reserved must be 0");
        assert(es_ar.bits.unusable || (es_ar.bits.reserved0 == 0 && es_ar.bits.reserved1 == 0), "Guest ES Access Rights Reserved must be 0");
        assert(fs_ar.bits.unusable || (fs_ar.bits.reserved0 == 0 && fs_ar.bits.reserved1 == 0), "Guest FS Access Rights Reserved must be 0");
        assert(gs_ar.bits.unusable || (gs_ar.bits.reserved0 == 0 && gs_ar.bits.reserved1 == 0), "Guest GS Access Rights Reserved must be 0");

        assert(!(is_ia32e_mode && cs_ar.bits.long_mode) || cs_ar.bits.default_db == 0, "Guest CS Access Rights D/B must be 0");

        assert(((cs_limit & 0xfff) != 0xfff && cs_ar.bits.granularity == 0) || ((cs_limit & 0xfff00000) != 0xfff00000 && cs_ar.bits.granularity == 1), "Guest CS Access Rights Bad Granularity");
        assert(ds_ar.bits.unusable || (((ds_limit & 0xfff) != 0xfff && ds_ar.bits.granularity == 0) || ((ds_limit & 0xfff00000) != 0xfff00000 && ds_ar.bits.granularity == 1)), "Guest DS Access Rights Bad Granularity");
        assert(ss_ar.bits.unusable || (((ss_limit & 0xfff) != 0xfff && ss_ar.bits.granularity == 0) || ((ss_limit & 0xfff00000) != 0xfff00000 && ss_ar.bits.granularity == 1)), "Guest SS Access Rights Bad Granularity");
        assert(es_ar.bits.unusable || (((es_limit & 0xfff) != 0xfff && es_ar.bits.granularity == 0) || ((es_limit & 0xfff00000) != 0xfff00000 && es_ar.bits.granularity == 1)), "Guest ES Access Rights Bad Granularity");
        assert(fs_ar.bits.unusable || (((fs_limit & 0xfff) != 0xfff && fs_ar.bits.granularity == 0) || ((fs_limit & 0xfff00000) != 0xfff00000 && fs_ar.bits.granularity == 1)), "Guest FS Access Rights Bad Granularity");
        assert(gs_ar.bits.unusable || (((gs_limit & 0xfff) != 0xfff && gs_ar.bits.granularity == 0) || ((gs_limit & 0xfff00000) != 0xfff00000 && gs_ar.bits.granularity == 1)), "Guest GS Access Rights Bad Granularity");
    }

    {
        const auto tr_ar = x86::vmx::segment_access_rights_t{static_cast<uint32_t>(tr_access_rights_raw)};

        assert(is_ia32e_mode || is_any_of(tr_ar.bits.type, 3, 11), "Guest TR Access Rights Type invalid");
        assert(!is_ia32e_mode || tr_ar.bits.type == 11, "Guest TR Access Rights Type invalid");
        assert(tr_ar.bits.s == 0, "Guest TR Access Rights S must be 0");
        assert(tr_ar.bits.present, "Guest TR Access Rights P must be 1");
        assert(tr_ar.bits.unusable == 0, "Guest TR Access Rights must be usable");
        assert(((tr_limit & 0xfff) != 0xfff && tr_ar.bits.granularity == 0) || ((tr_limit & 0xfff00000) != 0xfff00000 && tr_ar.bits.granularity == 1), "Guest TR Access Rights Bad Granularity");
        assert(tr_ar.bits.reserved0 == 0 && tr_ar.bits.reserved1 == 0, "Guest TR Access Rights Reserved must be 0");
    }

    {
        const auto ldtr_ar = x86::vmx::segment_access_rights_t{static_cast<uint32_t>(ldtr_access_rights_raw)};
        if (!ldtr_ar.bits.unusable) {
            assert(ldtr_ar.bits.type == 2, "Guest LDTR Access Rights Type must be 2");
            assert(ldtr_ar.bits.s == 0, "Guest LDTR Access Rights S must be 0");
            assert(ldtr_ar.bits.present == 1, "Guest LDTR Access Rights P must be 1");
            assert(((ldtr_limit & 0xfff) != 0xfff && ldtr_ar.bits.granularity == 0) || ((tr_limit & 0xfff00000) != 0xfff00000 && ldtr_ar.bits.granularity == 1), "Guest LDTR Access Rights Bad Granularity");
            assert(ldtr_ar.bits.reserved0 == 0 && ldtr_ar.bits.reserved1, "Guest LDTR Access Rights Reserved must be 0");
        }
    }

    {
        uint64_t gdtr_base;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_gdtr_base, gdtr_base));
        uint64_t gdtr_limit;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_gdtr_limit, gdtr_limit));
        uint64_t idtr_base;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_idtr_base, idtr_base));
        uint64_t idtr_limit;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_idtr_limit, idtr_limit));

        if (is_cpu_intel64) {
            assert(x86::paging::is_canonical(gdtr_base), "Guest GDTR Base not canonical");
            assert(x86::paging::is_canonical(idtr_base), "Guest iDTR Base not canonical");
        }

        assert((gdtr_limit & 0xffff0000) == 0, "Guest GDTR Limit bits 31:16 must be 0");
        assert((idtr_limit & 0xffff0000) == 0, "Guest IDTR Limit bits 31:16 must be 0");
    }

    return {};
}

framework::result<> do_vm_entry_checks() {
    trace_debug("Starting VMENTRY Checks");

    verify(verify_vmentry_exec_control_fields());
    verify(verify_vmentry_exit_control_fields());
    verify(verify_vmentry_entry_control_fields());
    verify(verify_host_state());
    verify(verify_guest_registers());
    verify(verify_guest_segments());

    // todo: add guest state checks SDM 26.3.1 P1106
    //      need to add non-register state checks (26.3.1.5, P1110)

    trace_debug("Done VMENTRY Checks");

    return {};
}

}
