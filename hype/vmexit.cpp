
#include <x86/vmx/vmcs.h>
#include <x86/vmx/vmexit.h>
#include <x86/vmx/vmx.h>
#include <x86/cpuid.h>

#include "base.h"
#include "context.h"
#include "cpu.h"
#include "memory.h"
#include "vmentry.h"

namespace hype {

static framework::result<> handle_cpuid(cpu_registers_t& registers) {
    const auto cpuid = x86::cpuid(registers.rax, registers.rcx);

    trace_debug("CPUID[rax=0x%lx, rcx=0x%lx]: eax=0x%lx, ebx=0x%lx, ecx=0x%lx, edx=0x%lx",
        registers.rax, registers.rcx, cpuid.eax, cpuid.ebx, cpuid.ecx, cpuid.edx);

    registers.rax = cpuid.eax;
    registers.rbx = cpuid.ebx;
    registers.rcx = cpuid.ecx;
    registers.rdx = cpuid.edx;

    return {};
}

framework::result<> handle_vmexit(cpu_registers_t& registers) {
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_rip, registers.rip));

    uint64_t exit_reason_raw;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::exit_reason, exit_reason_raw));

    const auto exit_reason = static_cast<x86::vmx::exit_reason_t>(exit_reason_raw & 0xffff);
    trace_debug("Exit %u From 0x%llx", static_cast<uint16_t>(exit_reason), registers.rip);

    {
        // todo: limits are fucked post exit, maybe a result of restoration
        //  via iretq
        x86::vmx::vmwrite(x86::vmx::field_t::guest_cs_limit, 0xfffff);
        x86::vmx::vmwrite(x86::vmx::field_t::guest_ss_limit, 0xfffff);
    }

    switch (exit_reason) {
        case x86::vmx::exit_reason_t::cpuid:
            verify(handle_cpuid(registers));
            break;
        default:
            return framework::err(framework::status_unsupported);
    }

    // move guest to next instruction
    // todo: sometimes we don't want to do this, let handlers choose
    uint64_t instruction_len;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::vmexit_instruction_length, instruction_len));
    registers.rip += instruction_len;
    verify_vmx(x86::vmx::vmwrite(x86::vmx::field_t::guest_rip, registers.rip));

    verify(do_vm_entry_checks());
    registers.rip = reinterpret_cast<uint64_t>(x86::vmx::vmresume);

    trace_debug("Resume guest into rip=0x%x", registers.rip);
    asm_cpu_load_registers(&registers);
}

}

extern "C" [[noreturn]] void vm_exit_handler(hype::cpu_registers_t& registers) {
    const auto status = handle_vmexit(registers);
    trace_status(status.error(), "Error from VMEXIT handler!");
    abort("failed to handle vmexit");
}
