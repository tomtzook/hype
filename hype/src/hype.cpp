
#include <x86/cpuid.h>
#include <x86/segment.h>
#include <x86/cr.h>
#include <x86/msr.h>
#include <x86/memory.h>
#include <x86/vmx/environment.h>
#include <x86/vmx/operations.h>

#include "common.h"

#include "hype.h"


struct hype::context_t {
    void* vmxon_region;

    bool is_in_vmx_operation;
};


static common::result check_environment_support() noexcept {
    if (!x86::vmx::is_supported()) {
        return hype::result::VMX_NOT_SUPPORTED;
    }
    // TODO: add check for secondary and primary controls
    // TODO: add check for unrestricted guest support

    return hype::result::SUCCESS;
}


common::result hype::initialize(context_t*&context) noexcept {
    common::result status = hype::result::SUCCESS;
    if (nullptr != context) {
        return hype::result::ALREADY_INITIALIZED;
    }

    CHECK(check_environment_support());

    // TODO: identify all logical processors

    context = new hype::context_t;

    // TODO: initialize per-cpu contexts
    // TODO: initialize page tables
    // TODO: initialize EPT
cleanup:
    return status;
}

common::result hype::start(context_t* context) noexcept {
    common::result status = hype::result::SUCCESS;

    // TODO: needs to be done for each processor

    CHECK(x86::vmx::on(context->vmxon_region));
    context->is_in_vmx_operation = true;

    // TODO: setup vmcs
    // TODO: load vmcs
    // TODO: launch

cleanup:
    return status;
}

void hype::free(context_t* context) noexcept {
    if (context->is_in_vmx_operation) {
        CHECK_SILENT(x86::vmx::off());
    }

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
        default: return L"";
    }
}
#endif
