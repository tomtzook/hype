
#include <x86/paging/ia32e.h>
#include <x86/vmx/vmx.h>
#include <x86/msr.h>
#include <x86/vmx/controls.h>

#include "base.h"
#include "memory.h"
#include "context.h"
#include "vmx.h"

#include "hype.h"


namespace hype {

context_t* g_context = nullptr;

struct wanted_vm_controls_t {
    x86::vmx::pin_based_exec_controls_t pinbased;
    x86::vmx::processor_based_exec_controls_t procbased;
    x86::vmx::secondary_processor_based_exec_controls_t secondary_procbased;
    x86::vmx::vmexit_controls_t vmexit;
    x86::vmx::vmentery_controls_t vmentry;
};

wanted_vm_controls_t get_wanted_vm_controls() noexcept {
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
    CHECK_ASSERT(x86::vmx::is_supported(), "vmx not supported");

    auto vmx_basic = x86::read<x86::msr::ia32_vmx_basic_t>();
    CHECK_ASSERT(vmx_basic.bits.vm_ctrls_fixed == 1,
                 "VMX True MSR Controls not supported");

    CHECK_ASSERT(check_wanted_vm_controls(),
                 "Wanted VM Controls not supported");

    CHECK_ASSERT(x86::paging::mode_t::ia32e == x86::paging::current_mode() &&
                 x86::paging::ia32e::are_huge_tables_supported(),
                 "required paging not supported");

    return {};
}


static status_t start_on_vcpu(void*) noexcept {
    environment::set_current_vcpu_id(0); // TODO: VALUE?

    auto& cpu = get_current_vcpu();
    cpu.is_in_vmx_operation = false;

    TRACE_DEBUG("Starting on core");

    CHECK(vmxon_for_vcpu(cpu));
    cpu.is_in_vmx_operation = true;

    auto vmcs_physical = environment::to_physical(&cpu.vmcs);
    CHECK_VMX(x86::vmx::vmclear(vmcs_physical));
    CHECK(setup_vmcs(cpu));
    CHECK_VMX(x86::vmx::vmptrld(vmcs_physical));
    CHECK_VMX(x86::vmx::vmlaunch());

    return {};
}

static status_t stop_on_vcpu(void*) noexcept {
    auto& cpu = get_current_vcpu();

    if (cpu.is_in_vmx_operation) {
        x86::vmx::vmxoff();
        cpu.is_in_vmx_operation = false;
    }

    return {};
}

status_t initialize() noexcept {
    CHECK_ASSERT(g_context == nullptr, "context not null");

    TRACE_DEBUG("Checking environment support");
    CHECK(check_environment_support());

    TRACE_DEBUG("Initializing context");
    g_context = new (x86::paging::page_size) context_t;
    CHECK_ALLOCATION(g_context, "context initialization failed");

    status_t status{};
    CHECK_AND_JUMP(cleanup, status, memory::setup_identity_paging(g_context->page_table));
    // todo: initialize ept

cleanup:
    if (!status && g_context != nullptr) {
        delete g_context;
        g_context = nullptr;
    }

    return status;
}

status_t start() noexcept {
    CHECK(environment::run_on_all_vcpu(start_on_vcpu, nullptr));
    return {};
}

void free() noexcept {
    environment::run_on_all_vcpu(stop_on_vcpu, nullptr);

    if (g_context != nullptr) {
        delete g_context;
        g_context = nullptr;
    }
}

}
