
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
};


static common::result check_environment_support() noexcept {
    if (!x86::vmx::is_supported()) {
        return hype::result::VMX_NOT_SUPPORTED;
    }

    return hype::result::SUCCESS;
}


common::result hype::initialize(context_t*&context) noexcept {
    common::result status = hype::result::SUCCESS;
    if (nullptr != context) {
        return hype::result::ALREADY_INITIALIZED;
    }

    CHECK(check_environment_support());

    context = new hype::context_t;
cleanup:
    return status;
}

common::result hype::start(context_t* context) noexcept {
    common::result status = hype::result::SUCCESS;

    CHECK(x86::vmx::on(context->vmxon_region));

cleanup:
    return status;
}

common::result hype::free(context_t* context) noexcept {
    ::operator delete(context);
    return hype::result::SUCCESS;
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

/*x86::cpuid_regs_t cpuid_regs = {};
x86::cpu_id(1, cpuid_regs);
TRACE_CPUID_REG(1, 0, "EDX", cpuid_regs.edx);

auto eax_01 = x86::cpu_id<x86::cpuid_eax01_t>(1);
TRACE_CPUID_BIT(1, 0, "EDX", "PAE", eax_01.edx.bits.pae);

x86::segment_table_t table{};
x86::store(table);
hype::debug::trace(table);

x86::cr0_t cr0;
x86::write(cr0);
TRACE_REG_BIT("CR0", "PE", cr0.bits.paging_enable);

auto efer = x86::msr::read<x86::msr::ia32_efer_t>(x86::msr::ia32_efer_t::ID);
TRACE_REG_BIT("EFER", "LMA", efer.bits.lma);

auto* cr0_n = new x86::cr0_t(0);
VERIFY_ALLOCATION(cr0_n);
TRACE_DEBUG("ALLOCATION s=%x", cr0_n);
TRACE_DEBUG("plop");
operator delete(cr0_n);*/