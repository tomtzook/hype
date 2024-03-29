
#include <x86/paging/ia32e.h>
#include <x86/vmx/vmx.h>
#include <x86/msr.h>
#include <x86/vmx/controls.h>
#include <x86/segments.h>
#include <x86/mtrr.h>

#include "base.h"
#include "memory.h"
#include "context.h"
#include "vmx.h"

#include "hype.h"


namespace hype {

context_t* g_context = nullptr;

static wanted_vm_controls_t get_wanted_vm_controls() noexcept {
    wanted_vm_controls_t controls{};
    controls.procbased.bits.activate_secondary_controls = true;
    controls.procbased.bits.use_io_bitmaps = true;
    controls.procbased.bits.use_msr_bitmaps = true;
    controls.secondary_procbased.bits.enable_ept = true;
    controls.secondary_procbased.bits.enable_xsaves_xstors = true;
    controls.secondary_procbased.bits.unrestricted_guest = true;

    return controls;
}

static status_t check_wanted_vm_controls() noexcept {
    auto controls = get_wanted_vm_controls();

    CHECK_ASSERT(x86::vmx::are_vm_controls_supported(controls.pinbased),
                 "wanted pin based controls not supported");
    CHECK_ASSERT(x86::vmx::are_vm_controls_supported(controls.procbased),
                 "wanted processor based controls not supported");
    CHECK_ASSERT(x86::vmx::are_vm_controls_supported(controls.secondary_procbased),
                 "wanted secondary processor based controls not supported");
    CHECK_ASSERT(x86::vmx::are_vm_controls_supported(controls.vmexit),
                 "wanted vmexit controls not supported");
    CHECK_ASSERT(x86::vmx::are_vm_controls_supported(controls.vmentry),
                 "wanted vmentry controls not supported");

    return {};
}

static status_t check_environment_support() noexcept {
    // todo: check intel

    CHECK_ASSERT(x86::vmx::is_supported(), "vmx not supported");

    auto vmx_basic = x86::read<x86::msr::ia32_vmx_basic_t>();
    CHECK_ASSERT(vmx_basic.bits.vm_ctrls_fixed == 1,
                 "VMX True MSR Controls not supported");

    auto ept_cap = x86::read<x86::msr::ia32_vmx_ept_vpid_cap_t>();
    CHECK_ASSERT(ept_cap.bits.ept_large_pages && ept_cap.bits.invept && ept_cap.bits.memory_type_write_back,
                 "Wanted EPT/VPID features not supported");

    CHECK_ASSERT(check_wanted_vm_controls(),
                 "Wanted VM Controls not supported");

    CHECK_ASSERT(x86::paging::mode_t::ia32e == x86::paging::current_mode() &&
                 x86::paging::ia32e::are_huge_tables_supported(),
                 "required paging not supported");

    // todo: check mtrr supported

    return {};
}


static status_t start_on_vcpu(void*) noexcept {
    auto cpu_id = x86::atomic::fetchadd8(&g_context->cpu_init_index, 1);
    environment::set_current_vcpu_id(cpu_id);

    TRACE_DEBUG("Starting on core id=0x%x", cpu_id);

    auto& cpu = get_current_vcpu();
    cpu.is_in_vmx_operation = false;

    // todo: setup gdt with tss
    // todo: setup idt

    CHECK(vmxon_for_vcpu(cpu));
    cpu.is_in_vmx_operation = true;

    TRACE_DEBUG("Initializing vmcs");
    auto vmcs_physical = environment::to_physical(&cpu.vmcs);
    CHECK_VMX(x86::vmx::vmclear(vmcs_physical));
    CHECK_ASSERT(x86::vmx::initialize_vmstruct(cpu.vmcs), "initialize_vmstruct failed");
    CHECK_VMX(x86::vmx::vmptrld(vmcs_physical));
    CHECK(setup_vmcs(*g_context, cpu));

    TRACE_DEBUG("Doing vmlaunch");
    CHECK_VMX(x86::vmx::vmlaunch());

    return {};
}

static status_t stop_on_vcpu(void*) noexcept {
    auto& cpu = get_current_vcpu();

    TRACE_DEBUG("Running stop on core");

    if (cpu.is_in_vmx_operation) {
        TRACE_DEBUG("Doing vmxoff on core");
        x86::vmx::vmxoff();
        cpu.is_in_vmx_operation = false;
    }

    return {};
}

status_t initialize() noexcept {
    CHECK_ASSERT(g_context == nullptr, "context not null");

    TRACE_DEBUG("Checking environment support");
    CHECK(check_environment_support());

    auto gdtr = x86::segments::table_t(x86::read<x86::segments::gdtr_t>());
    for (int i = 0; i < gdtr.count(); ++i) {
        auto segment = gdtr[i];
        TRACE_DEBUG("SEGMENT: i=0x%x, base=0x%x, limit=0x%x, s=0x%x, type=0x%x",
                    i,
                    segment.base_address(), segment.limit(),
                    segment.bits.s, segment.bits.type);
    }

    // todo: setup gdt

    auto mtrr_cache = x86::mtrr::initialize_cache();

    TRACE_DEBUG("Initializing context");
    g_context = new (x86::paging::page_size) context_t;
    CHECK_ALLOCATION(g_context, "context initialization failed");

    g_context->wanted_vm_controls = get_wanted_vm_controls();
    g_context->cpu_init_index = 0;

    status_t status{};
    CHECK_AND_JUMP(cleanup, status, environment::get_active_cpu_count(g_context->cpu_count));

    TRACE_DEBUG("Initializing Page Table");
    CHECK_AND_JUMP(cleanup, status, memory::setup_identity_paging(g_context->page_table));

    TRACE_DEBUG("Initializing EPT");
    CHECK_AND_JUMP(cleanup, status, memory::setup_identity_ept(g_context->ept, mtrr_cache));

    TRACE_DEBUG("loading page table");
    // todo: this causes problems for freepages, we need to modify the page table before allocation?
    {
        auto cr3 = x86::read<x86::cr3_t>();
        cr3.ia32e.address = environment::to_physical(&g_context->page_table.m_pml4) >> x86::paging::page_bits_4k;

        x86::write(cr3);
    }

cleanup:
    if (!status && g_context != nullptr) {
        delete g_context;
        g_context = nullptr;
    }

    return status;
}

status_t start() noexcept {
    TRACE_DEBUG("Starting Hype on all cores");
    CHECK(environment::run_on_all_vcpu(start_on_vcpu, nullptr));
    return {};
}

void free() noexcept {
    TRACE_DEBUG("Doing free on all cores");
    // todo: only stop on cpus that ran
    environment::run_on_all_vcpu(stop_on_vcpu, nullptr);

    if (g_context != nullptr) {
        delete g_context;
        g_context = nullptr;
    }
}

}
