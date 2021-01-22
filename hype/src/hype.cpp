
#include <x86/cpuid.h>
#include <x86/segment.h>
#include <x86/cr.h>
#include <x86/msr.h>
#include <x86/memory.h>
#include <x86/paging.h>

#include "common.h"
#include "vcpu.h"
#include "env.h"

#include "hype.h"


struct hype::context_t {
    environment_t environment;

    x86::paging::huge_page_table_t page_table PAGE_ALIGNED;
};


static common::result check_environment_support() noexcept {
    if (!x86::vmx::is_supported()) {
        return hype::result::VMX_NOT_SUPPORTED;
    }
    // TODO: add check for secondary and primary controls
    // TODO: add check for unrestricted guest support

    if (!x86::paging::are_huge_tables_supported()) {
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

common::result hype::initialize(context_t*&context) noexcept {
    common::result status = hype::result::SUCCESS;
    if (nullptr != context) {
        return hype::result::ALREADY_INITIALIZED;
    }

    CHECK(check_environment_support());
    CHECK(initialize(context->environment));

    context = new hype::context_t;
    // TODO: initialize page tables
    CHECK(x86::paging::setup_identity_paging(context->page_table));
    // TODO: initialize EPT
cleanup:
    return status;
}

common::result hype::start(context_t* context) noexcept {
    common::result status = hype::result::SUCCESS;

    CHECK(context->environment.get_vcpu_service().run_on_each_vcpu(
            start_on_vcpu,
            context));
cleanup:
    return status;
}

void hype::free(context_t* context) noexcept {
    CHECK_SILENT(context->environment.get_vcpu_service().run_on_each_vcpu(
            free_on_vcpu,
            context));

    ::operator delete(context);
}

template<>
common::result::result_t(hype::result::code_t code) noexcept
        : m_code(code)
        , m_category(hype::result::CATEGORY)
{}

#ifdef _DEBUG
const wchar_t* hype::result::debug::to_string(const common::result& result) noexcept {
    switch (result.code()) {
        case hype::result::SUCCESS: return L"SUCCESS";
        case hype::result::VMX_NOT_SUPPORTED: return L"VMX_NOT_SUPPORTED";
        case hype::result::ALREADY_INITIALIZED: return L"ALREADY_INITIALIZED";
        case hype::result::HUGE_PAGES_NOT_SUPPORTED: return L"HUGE_PAGES_NOT_SUPPORTED";
        default: return L"";
    }
}
#endif
