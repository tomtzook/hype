
#include <x86/paging/ia32e.h>
#include <x86/vmx/vmx.h>

#include "base.h"
#include "memory.h"
#include "context.h"
#include "vmx.h"

#include "hype.h"


namespace hype {

context_t* g_context = nullptr;

static status_t check_environment_support() noexcept {
    CHECK_ASSERT(x86::vmx::is_supported(), "vmx not supported");

    // TODO: add check for secondary and primary controls
    // TODO: add check for unrestricted guest support

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
    CHECK_ASSERT(x86::vmx::vmclear(vmcs_physical), "vmclear failed");
    CHECK(setup_vmcs(cpu));
    CHECK_ASSERT(x86::vmx::vmptrld(vmcs_physical), "vmptrld failed");
    CHECK_ASSERT(x86::vmx::vmlaunch(), "vmlaunch failed");

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
