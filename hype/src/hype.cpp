
#include <x86/cpuid.h>
#include <x86/segment.h>
#include <x86/paging/paging.h>

#include "common.h"
#include "vcpu.h"
#include "env.h"

#include "hype.h"


hype::context_t* g_context = nullptr;


static common::result check_environment_support() noexcept {
    if (!x86::vmx::is_supported()) {
        return hype::result::VMX_NOT_SUPPORTED;
    }
    // TODO: add check for secondary and primary controls
    // TODO: add check for unrestricted guest support

    if (x86::paging::mode_t::IA32E_PAGING != x86::paging::get_mode() ||
        !x86::paging::are_huge_tables_supported()) {
        return hype::result::HUGE_PAGES_NOT_SUPPORTED;
    }

    return hype::result::SUCCESS;
}

static common::result start_on_vcpu(void* param) noexcept {
    common::result status;

    hype::context_t* context = reinterpret_cast<hype::context_t*>(param);
    hype::vcpu_t& cpu = context->environment.get_vcpu_service().get_current_vcpu();

    TRACE_DEBUG("Starting on core");

    CHECK(x86::vmx::on(cpu.vmxon_region));
    cpu.is_in_vmx_operation = true;

    cpu.vmcs.clear();
    // TODO: setup vmcs
    // TODO: load vmcs
    CHECK(x86::vmx::write(cpu.vmcs));
    // TODO: launch

cleanup:
    if (status && cpu.is_in_vmx_operation) {
        CHECK_SILENT(x86::vmx::off());
        cpu.is_in_vmx_operation = false;
    }

    return status;
}

static common::result free_on_vcpu(void* param) noexcept {
    common::result status;

    hype::context_t* context = reinterpret_cast<hype::context_t*>(param);
    hype::vcpu_t& cpu = context->environment.get_vcpu_service().get_current_vcpu();

    if (cpu.is_in_vmx_operation) {
        CHECK(x86::vmx::off());
        cpu.is_in_vmx_operation = false;
    }

cleanup:
    return status;
}

common::result hype::initialize() noexcept {
    common::result status;
    if (nullptr != g_context) {
        return hype::result::ALREADY_INITIALIZED;
    }

    CHECK(check_environment_support());
    TRACE_DEBUG("Environment supported");

    g_context = new (environment::alignment_t::PAGE_ALIGN) hype::context_t;
    CHECK_ALLOCATION(g_context);

    CHECK(initialize(g_context->environment));
    CHECK(x86::paging::setup_identity_paging(g_context->page_table));
    // TODO: initialize EPT

    TRACE_DEBUG("Context initialized");
cleanup:
    if (!status) {
        ::operator delete(g_context);
    }
    return status;
}

common::result hype::start() noexcept {
    common::result status;

    CHECK(g_context->environment.get_vcpu_service().run_on_each_vcpu(
            start_on_vcpu,
            g_context));
cleanup:
    return status;
}

void hype::free() noexcept {
    CHECK_SILENT(g_context->environment.get_vcpu_service().run_on_each_vcpu(
            free_on_vcpu,
            g_context));

    ::operator delete(g_context);
}

template<>
common::result::result_t(hype::result::code_t code) noexcept
        : m_code(code)
        , m_category(hype::result::CATEGORY)
{}
