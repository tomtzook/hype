
#include <x86/paging/ia32e.h>
#include <x86/vmx/vmx.h>
#include <x86/msr.h>
#include <x86/vmx/controls.h>
#include <x86/segments.h>
#include <x86/mtrr.h>
#include <x86/cpuid.h>
#include <ptr.h>

#include "base.h"
#include "cpu.h"
#include "memory.h"
#include "context.h"
#include "vmx.h"
#include "vmentry.h"

#include "hype.h"


namespace hype {

context_t* g_context = nullptr;

static wanted_vm_controls_t get_wanted_vm_controls() {
    wanted_vm_controls_t controls{};
    controls.procbased.bits.activate_secondary_controls = true;
    //controls.procbased.bits.use_io_bitmaps = true;
    //controls.procbased.bits.use_msr_bitmaps = true;
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

    assert(x86::vmx::is_supported(), "vmx not supported");

    const auto vmx_basic = x86::read<x86::msr::ia32_vmx_basic_t>();
    assert(vmx_basic.bits.vm_ctrls_fixed == 1,
                 "VMX True MSR Controls not supported");

    const auto ept_cap = x86::read<x86::msr::ia32_vmx_ept_vpid_cap_t>();
    assert(ept_cap.bits.ept_large_pages && ept_cap.bits.invept && ept_cap.bits.memory_type_write_back,
                 "Wanted EPT/VPID features not supported");

    assert(check_wanted_vm_controls(),
                 "Wanted VM Controls not supported");

    assert(x86::paging::mode_t::ia32e == x86::paging::current_mode() &&
                 x86::paging::ia32e::are_huge_tables_supported(),
                 "required paging not supported");

    // todo: check mtrr supported

    return {};
}

static framework::result<> init_context(context_t* context, const x86::mtrr::mtrr_cache_t& mtrr_cache) {
    context->wanted_vm_controls = get_wanted_vm_controls();
    context->cpu_init_index = 0;
    context->cpu_count = verify(framework::environment::get_active_cpu_count());

    trace_debug("Initializing GDT");
    verify(memory::setup_gdt(context->gdtr, context->gdt, context->tss));
    trace_debug("Initializing IDT");
    verify(interrupts::setup_idt(context->idtr, context->idt));
    trace_debug("Initializing Page Table");
    verify(memory::setup_identity_paging(context->page_table));
    trace_debug("Initializing EPT");
    verify(memory::setup_identity_ept(context->ept, mtrr_cache));

    return {};
}

static framework::result<> start_on_vcpu(void*) {
    const auto cpu_id = x86::atomic::fetchadd8(&g_context->cpu_init_index, 1);
    framework::environment::set_current_vcpu_id(cpu_id);

    trace_debug("Starting on core id=0x%x", cpu_id);

    auto& cpu = get_current_vcpu();
    cpu.is_in_vmx_operation = false;

    asm_cpu_store_registers(&cpu.context_registers);
    // if operation is on, then we were returned here from the registers being restored,
    // meaning we launched.
    if (cpu.is_in_vmx_operation) {
        x86::cpuid(22, 1);
        return {};
    }

    trace_debug("Entering VMX");
    verify(vmxon_for_vcpu(cpu));
    cpu.is_in_vmx_operation = true;

    trace_debug("Initializing vmcs");
    const auto vmcs_physical = framework::environment::to_physical(&cpu.vmcs);
    verify_vmx(x86::vmx::vmclear(vmcs_physical));
    assert(x86::vmx::initialize_vmstruct(cpu.vmcs), "initialize_vmstruct failed");
    verify_vmx(x86::vmx::vmptrld(vmcs_physical));
    verify(setup_vmcs(*g_context, cpu));

    verify(do_vm_entry_checks());

    trace_debug("Doing vmlaunch");
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
    assert(g_context == nullptr, "context not null");

    trace_debug("Checking environment support");
    verify(check_environment_support());

    trace_debug("Setting up new GDT");
    verify(memory::setup_initial_guest_gdt());

    auto mtrr_cache = x86::mtrr::initialize_cache();

    trace_debug("Initializing context");
    auto context = verify(framework::unique_ptr<context_t>::create<x86::paging::page_size>());
    verify_alloc(context);

    context->wanted_vm_controls = get_wanted_vm_controls();
    context->cpu_init_index = 0;
    context->cpu_count = verify(framework::environment::get_active_cpu_count());
    const auto context_result = init_context(context.get(), mtrr_cache);
    if (context_result) {
        g_context = context.release();
    }

    return {};
}

framework::result<> start() {
    trace_debug("Starting Hype on all cores");
    verify(framework::environment::run_on_all_vcpu(start_on_vcpu, nullptr));

    return {};
}

void free() {
    trace_debug("Doing free on all cores");

    framework::debug::deadloop();

    // todo: only stop on cpus that ran
    framework::environment::run_on_all_vcpu(stop_on_vcpu, nullptr);

    if (g_context != nullptr) {
        delete g_context;
        g_context = nullptr;
    }
}

}
