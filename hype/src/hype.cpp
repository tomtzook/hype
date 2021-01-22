
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
    common::result status = hype::result::SUCCESS;

    hype::context_t* context = reinterpret_cast<hype::context_t*>(param);
    hype::vcpu_t& cpu = context->environment.get_vcpu_service().get_current_vcpu();

    {
        physical_address_t vmxon_physaddress;
        CHECK(x86::vmx::initialize_vmxon_region(cpu.vmxon_region));
        vmxon_physaddress = environment::to_physical(&cpu.vmxon_region);
        CHECK(x86::vmx::on(vmxon_physaddress));
    }

    cpu.is_in_vmx_operation = true;

    // TODO: setup vmcs
    // TODO: load vmcs
    // TODO: launch

cleanup:
    return status;
}

static common::result free_on_vcpu(void* param) noexcept {
    common::result status = hype::result::SUCCESS;

    hype::context_t* context = reinterpret_cast<hype::context_t*>(param);
    hype::vcpu_t& cpu = context->environment.get_vcpu_service().get_current_vcpu();

    CHECK(x86::vmx::off());
    cpu.is_in_vmx_operation = false;

cleanup:
    return status;
}

common::result hype::initialize() noexcept {
    common::result status = hype::result::SUCCESS;
    if (nullptr != g_context) {
        return hype::result::ALREADY_INITIALIZED;
    }

    CHECK(check_environment_support());

    g_context = new hype::context_t;
    CHECK_ALLOCATION(g_context);

    CHECK(initialize(g_context->environment));
    CHECK(x86::paging::setup_identity_paging(g_context->page_table));
    // TODO: initialize EPT
cleanup:
    if (!status) {
        ::operator delete(g_context);
    }
    return status;
}

common::result hype::start() noexcept {
    common::result status = hype::result::SUCCESS;

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

DEBUG_DECL(hype::result,
const wchar_t* to_string(const common::result& result) noexcept {
    switch (result.code()) {
        case hype::result::SUCCESS: return L"SUCCESS";
        case hype::result::VMX_NOT_SUPPORTED: return L"VMX_NOT_SUPPORTED";
        case hype::result::ALREADY_INITIALIZED: return L"ALREADY_INITIALIZED";
        case hype::result::HUGE_PAGES_NOT_SUPPORTED: return L"HUGE_PAGES_NOT_SUPPORTED";
        default: return L"";
    }
}
)
