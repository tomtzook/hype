
#include <x86/cpuid.h>
#include <x86/segment.h>
#include <x86/cr.h>
#include <x86/msr.h>
#include <x86/memory.h>
#include <x86/vmx/environment.h>

#include "common.h"

#include "hype.h"


struct hype::context_t {

};


static common::result check_environment_support() noexcept {
    if (!x86::vmx::is_supported()) {
        return hype::result::NOT_SUPPORTED;
    }

    return hype::result::SUCCESS;
}


common::result hype::initialize(context_t*&context) noexcept {
    common::result status = hype::result::SUCCESS;
    if (nullptr != context) {
        return hype::result::SUCCESS;
    }

    CHECK(check_environment_support());

    context = new hype::context_t;
cleanup:
    return status;
}

common::result hype::free(context_t* context) noexcept {
    ::operator delete(context);
    return hype::result::SUCCESS;
}


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