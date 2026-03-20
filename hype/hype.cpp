
#include <x86/paging/ia32e.h>
#include <x86/vmx/vmx.h>
#include <x86/msr.h>
#include <x86/vmx/controls.h>
#include <x86/mtrr.h>
#include <x86/apic.h>
#include <x86/id.h>
#include <x86/regs.h>

#include <base.h>
#include "cpu.h"
#include "memory.h"
#include "context.h"
#include "vmx/vmx.h"
#include "vmx/vmentry.h"
#include "debug.h"
#include "hype.h"

namespace hype {

context_t g_context;

static wanted_vm_controls_t get_wanted_vm_controls() {
    wanted_vm_controls_t controls{};
    controls.procbased.bits.activate_secondary_controls = true;
    //controls.procbased.bits.use_io_bitmaps = true;
    controls.procbased.bits.use_msr_bitmaps = true;
    controls.secondary_procbased.bits.enable_ept = true;
    controls.secondary_procbased.bits.enable_xsaves_xstors = true;
    controls.secondary_procbased.bits.unrestricted_guest = true;
    controls.vmentry.bits.ia32e_mode_guest = true;
    controls.vmexit.bits.host_address_space_size = true;

    return controls;
}

static framework::result<> check_wanted_vm_controls() {
    const auto controls = get_wanted_vm_controls();

    assert(x86::vmx::are_vm_controls_supported(controls.pinbased),
                 "wanted pin based controls not supported");
    assert(x86::vmx::are_vm_controls_supported(controls.procbased),
                 "wanted processor based controls not supported");
    assert(x86::vmx::are_vm_controls_supported(controls.secondary_procbased),
                 "wanted secondary processor based controls not supported");
    assert(x86::vmx::are_vm_controls_supported(controls.vmexit),
                 "wanted vmexit controls not supported");
    assert(x86::vmx::are_vm_controls_supported(controls.vmentry),
                 "wanted vmentry controls not supported");

    return {};
}

static framework::result<> check_environment_support() {
    // todo: check intel

    assert(x86::is_intel_cpu(), "cpu not intel");
    assert(x86::apic::is_x2apic_supported(), "x2apic not supported");
    assert(x86::paging::mode_t::ia32e == x86::paging::current_mode() &&
                     x86::paging::ia32e::are_huge_tables_supported(),
                     "required paging not supported");

    assert(x86::vmx::is_supported(), "vmx not supported");

    const auto vmx_basic = x86::read<x86::msr::ia32_vmx_basic_t>();
    assert(vmx_basic.bits.vm_ctrls_fixed == 1, "VMX True MSR Controls not supported");

    const auto ept_cap = x86::read<x86::msr::ia32_vmx_ept_vpid_cap_t>();
    assert(ept_cap.bits.ept_large_pages && ept_cap.bits.invept && ept_cap.bits.memory_type_write_back,
                 "Wanted EPT/VPID features not supported");

    assert(check_wanted_vm_controls(), "Wanted VM Controls not supported");

    // todo: check mtrr supported

    return {};
}

static framework::result<> init_context(context_t& context, const x86::mtrr::mtrr_cache_t& mtrr_cache) {
    context.wanted_vm_controls = get_wanted_vm_controls();
    context.cpu_init_index = 0;
    context.cpu_count = verify(environment::get_active_cpu_count());

    trace_debug("Initializing IDT");
    verify(interrupts::setup_idt(context.idtr, context.idt));
    trace_debug("Initializing Page Table");
    verify(memory::setup_identity_paging(context.page_table));
    trace_debug("Initializing EPT");
    verify(memory::setup_identity_ept(context.ept, mtrr_cache));

    trace_debug("Mapping Stack Guard");
    context.stack_guard.map_into_pml4e(context.page_table, memory::page_table_t::stack_guard_pml4e);

    trace_debug("Loading Base Modules");
    const auto image_info = verify(environment::get_our_image_info());
    context.loaded_modules.add_if_image_base(image_info.base);

    trace_debug("Done context init");

    return {};
}

static framework::result<> start_on_vcpu(void*) {
    const auto cpu_id = x86::atomic::fetchadd8(&g_context.cpu_init_index, 1);
    environment::set_current_vcpu_id(cpu_id);

    trace_debug("Starting on core id=0x%x (apic_id=0x%x)", cpu_id, x86::apic::get_local_apic_id());

    auto& cpu = get_current_vcpu();
    cpu.is_in_vmx_operation = false;

    trace_debug("Setting up new GDT");
    verify(memory::setup_initial_guest_gdt());

    trace_debug("Initializing GDT");
    verify(memory::setup_gdt(cpu.gdtr, cpu.gdt, cpu.tss));
    cpu.tss.ist[interrupts::idt_t::ist_index - 1] = reinterpret_cast<uint64_t>(cpu.interrupt_stack.end()) - vcpu_t::stack_shadow_space;

    g_context.stack_guard.create_guard(cpu.host_stack);
    memset(cpu.msr_bitmap, 0, sizeof(cpu.msr_bitmap));

    auto* initial_registers = reinterpret_cast<cpu_registers_t*>(reinterpret_cast<uint64_t>(cpu.guest_stack.end()) - vcpu_t::stack_shadow_space);
    asm_cpu_store_registers(initial_registers);
    // if operation is on, then we were returned here from the registers being restored,
    // meaning we launched.
    if (cpu.is_in_vmx_operation) {
        trace_debug("back from launch");
        return {};
    }

    trace_debug("Entering VMX");
    verify(vmxon_for_vcpu(cpu));
    cpu.is_in_vmx_operation = true;

    trace_debug("Initializing vmcs");
    const auto vmcs_physical = environment::to_physical(&cpu.vmcs);
    verify_vmx(x86::vmx::vmclear(vmcs_physical));
    assert(x86::vmx::initialize_vmstruct(cpu.vmcs), "initialize_vmstruct failed");
    verify_vmx(x86::vmx::vmptrld(vmcs_physical));
    verify(setup_vmcs(g_context, cpu));

    verify(do_vm_entry_checks());

    trace_debug("Launching");
    verify_vmx(x86::vmx::vmlaunch());

    return {};
}

static framework::result<> stop_on_vcpu(void*) {
    auto& cpu = get_current_vcpu();

    trace_debug("Running stop on core");

    if (cpu.is_in_vmx_operation) {
        trace_debug("Doing vmxoff on core");
        x86::vmx::vmxoff();
        cpu.is_in_vmx_operation = false;
    }

    return {};
}

framework::result<> initialize() {
    trace_debug("Checking environment support");
    verify(check_environment_support());

    const auto cpu_microarch = x86::get_microarchitecture(x86::get_cpu_model());
    const auto cpu_series = x86::microarchitecture_to_series(cpu_microarch);
    trace_debug("CPU is %a from %a", x86::cpu_microarchitecture_str(cpu_microarch), x86::cpu_series_str(cpu_series));

    assert(x86::apic::set_mode(x86::apic::mode_t::x2apic), "failed to enable x2apic");

    const auto mtrr_cache = x86::mtrr::initialize_cache();

    trace_debug("Initializing context");
    new (&g_context) context_t;
    verify(init_context(g_context, mtrr_cache));

    return {};
}

framework::result<> start() {
    trace_debug("Starting Hype on all cores");
    verify(environment::run_on_all_vcpu(start_on_vcpu, nullptr));

    return {};
}

void free() {
    trace_debug("Doing free on all cores");

    hype::deadloop();

    // todo: only stop on cpus that ran
    environment::run_on_all_vcpu(stop_on_vcpu, nullptr);
}

}
