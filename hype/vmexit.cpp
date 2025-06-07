
#include <x86/vmx/vmcs.h>
#include <x86/vmx/vmexit.h>
#include <x86/vmx/vmx.h>
#include <x86/cpuid.h>

#include "base.h"
#include "cpu.h"
#include "vmentry.h"

namespace hype {

static status_t handle_cpuid(cpu_registers_t& registers) {
    const auto cpuid = x86::cpuid(registers.rax, registers.rcx);

    TRACE_DEBUG("CPUID[rax=0x%lx, rcx=0x%lx]: eax=0x%lx, ebx=0x%lx, ecx=0x%lx, edx=0x%lx",
        registers.rax, registers.rcx, cpuid.eax, cpuid.ebx, cpuid.ecx, cpuid.edx);

    registers.rax = cpuid.eax;
    registers.rbx = cpuid.ebx;
    registers.rcx = cpuid.ecx;
    registers.rdx = cpuid.edx;

    return {};
}

status_t handle_vmexit(cpu_registers_t& registers) {
    CHECK_VMX(x86::vmx::vmread(x86::vmx::field_t::guest_rip, registers.rip));
    CHECK_VMX(x86::vmx::vmread(x86::vmx::field_t::guest_rsp, registers.rsp));
    CHECK_VMX(x86::vmx::vmread(x86::vmx::field_t::guest_rflags, registers.rflags));

    uint64_t exit_reason_raw;
    CHECK_VMX(x86::vmx::vmread(x86::vmx::field_t::exit_reason, exit_reason_raw));

    const auto exit_reason = static_cast<x86::vmx::exit_reason_t>(exit_reason_raw & 0xffff);
    TRACE_DEBUG("Exit %d From 0x%llx", exit_reason, registers.rip);

    {
        // todo: limits are fucked post exit, maybe a result of restoration
        //  via iretq
        x86::vmx::vmwrite(x86::vmx::field_t::guest_cs_limit, 0xfffff);
        x86::vmx::vmwrite(x86::vmx::field_t::guest_ss_limit, 0xfffff);
    }

    switch (exit_reason) {
        case x86::vmx::exit_reason_t::cpuid:
            CHECK(handle_cpuid(registers));
            break;
        default:
            return status::error_unsupported;
    }

    // move guest to next instruction
    // todo: sometimes we don't want to do this, let handlers choose
    uint64_t instruction_len;
    CHECK_VMX(x86::vmx::vmread(x86::vmx::field_t::vmexit_instruction_length, instruction_len));
    registers.rip += instruction_len;
    CHECK_VMX(x86::vmx::vmwrite(x86::vmx::field_t::guest_rip, registers.rip));

    TRACE_DEBUG("Resume guest");
    CHECK(do_vm_entry_checks());
    CHECK_VMX(x86::vmx::vmresume());

    return {};
}

}

extern "C" [[noreturn]] void vm_exit_handler(hype::cpu_registers_t& registers) {
    const auto status = handle_vmexit(registers);
    TRACE_ERROR("Error from VMEXIT handler!");
    TRACE_STATUS(status);

    //hype::debug::deadloop(); // TODO: this causes vm to pause (fault?)
    hype::hlt_cpu();
}
