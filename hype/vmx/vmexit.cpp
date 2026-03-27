
#include <x86/vmx/vmcs.h>
#include <x86/vmx/vmexit.h>
#include <x86/vmx/vmx.h>
#include <x86/cpuid.h>

#include <base.h>

#include "context.h"
#include "debug/modules.h"
#include "cpu.h"
#include "vmentry.h"

namespace hype {

static framework::result<> handle_exception_or_nmi(cpu_registers_t& registers) {
    uint64_t exit_info_raw;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::vmexit_interruption_information, exit_info_raw));
    const x86::vmx::vmexit_interruption_info_t exit_info{.raw = static_cast<uint32_t>(exit_info_raw)};
    uint64_t exit_interrupt_code;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::vmexit_interruption_error_code, exit_interrupt_code));

    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::idt_vectoring_information, exit_info_raw));
    const x86::vmx::vmexit_interruption_info_t idt_vectoring{.raw = static_cast<uint32_t>(exit_info_raw)};
    uint64_t idt_vectoring_code;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::idt_vectoring_error_code, idt_vectoring_code));

    const auto vector = static_cast<x86::interrupts::interrupt_t>(exit_info.bits.vector);
    trace_debug("EXCEPTION[vector=%a(0x%x)]: code=0x%x(%d)",
        x86::interrupts::vector_to_str(vector), static_cast<uint16_t>(vector),
        exit_interrupt_code, exit_info.bits.error_code_valid);

    // todo: handle NMI

    if (idt_vectoring.bits.valid) {
        // todo: handle idt vector info (exception while handling another)
        const auto nested_vector = static_cast<x86::interrupts::interrupt_t>(idt_vectoring.bits.vector);
        trace_debug("NESTED[vector=%a(0x%x)]:",
            x86::interrupts::vector_to_str(nested_vector), static_cast<uint16_t>(nested_vector),
            idt_vectoring_code, idt_vectoring.bits.error_code_valid);
    }

    // same struct for exit and entry
    verify_vmx(x86::vmx::vmwrite(x86::vmx::field_t::ctrl_vmentry_interruption_information_field, exit_info_raw));
    if (exit_info.bits.error_code_valid) {
        verify_vmx(x86::vmx::vmwrite(x86::vmx::field_t::ctrl_vmentry_exception_error_code, exit_interrupt_code));
    }
    if (x86::interrupts::vector_type(vector) == x86::interrupts::interrupt_type_t::trap) {
        uint64_t instruction_length;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::vmexit_instruction_length, instruction_length));
        verify_vmx(x86::vmx::vmwrite(x86::vmx::field_t::ctrl_vmentry_instruction_length, instruction_length));
    }

    return {};
}

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

static framework::result<> handle_rdmsr(cpu_registers_t& registers) {
    const auto id = registers.rcx & 0xffffffff;
    const auto value = x86::msr::read(id);

    trace_debug("RDMSR[0x%lx]: value=0x%lx", id, value);

    registers.rax = (value >> 32) & 0xffffffff;
    registers.rdx = value & 0xffffffff;

    return {};
}

static framework::result<> handle_wrmsr(const cpu_registers_t& registers) {
    const auto id = registers.rcx & 0xffffffff;
    const auto value = ((registers.rax & 0xffffffff) << 32) | (registers.rdx & 0xffffffff);

    trace_debug("WRMSR[0x%lx]: value=0x%lx", id, value);

    x86::msr::write(id, value);

    return {};
}

static framework::result<> handle_ept_violation(const cpu_registers_t& registers) {
    x86::vmx::ept_violation_exit_qualification_t exit_qualification{};
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::exit_qualification, exit_qualification.raw));

    uint64_t physical_address;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_physical_address, physical_address));
    uint64_t linear_address;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::exit_guest_linear_address, linear_address));

    trace_debug("EPTVIOLATION[physical=0x%p,linear=0x%p]: read=%d, write=%d, exec=%d",
        physical_address, linear_address,
        exit_qualification.bits.read_access, exit_qualification.bits.write_access, exit_qualification.bits.instruction_fetch);

    return framework::err(framework::status_unsupported);
}

framework::result<> handle_vmexit(cpu_registers_t& registers) {
    const auto old_rsp = registers.rsp;
    const auto old_rflags = registers.rflags;

    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_rip, registers.rip));
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_rsp, registers.rsp));
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_rflags, registers.rflags));

    auto& context = get_context();

    uint64_t exit_reason_raw;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::exit_reason, exit_reason_raw));

    const auto exit_reason = static_cast<x86::vmx::exit_reason_t>(exit_reason_raw & 0xffff);
    trace_debug("Exit %a (%u) From 0x%p", x86::vmx::exit_reason_str(exit_reason), static_cast<uint16_t>(exit_reason), registers.rip);
    debug::print_stack_frame(context.guest_memory_mapper, context.loaded_modules, registers.rip, registers.rbp, registers.rsp);

    {
        // todo: limits are fucked post exit, maybe a result of restoration
        //  via iretq
        x86::vmx::vmwrite(x86::vmx::field_t::guest_cs_limit, 0xfffff);
        x86::vmx::vmwrite(x86::vmx::field_t::guest_ss_limit, 0xfffff);
    }

    bool should_move_to_next_instruction = true;

    switch (exit_reason) {
        case x86::vmx::exit_reason_t::exception_or_nmi:
            verify(handle_exception_or_nmi(registers));
            should_move_to_next_instruction = false;
            break;
        case x86::vmx::exit_reason_t::cpuid:
            verify(handle_cpuid(registers));
            break;
        case x86::vmx::exit_reason_t::rdmsr:
            verify(handle_rdmsr(registers));
            break;
        case x86::vmx::exit_reason_t::wrmsr:
            verify(handle_wrmsr(registers));
            break;
        case x86::vmx::exit_reason_t::ept_violation:
            verify(handle_ept_violation(registers));
            should_move_to_next_instruction = false;
            break;
        default:
            trace_error("Unsupported exit %d", exit_reason);
            return framework::err(framework::status_unsupported);
    }

    // ReSharper disable once CppDFAConstantConditions
    if (should_move_to_next_instruction) {
        // move guest to next instruction
        uint64_t instruction_len;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::vmexit_instruction_length, instruction_len));
        registers.rip += instruction_len;
    }

    verify_vmx(x86::vmx::vmwrite(x86::vmx::field_t::guest_rip, registers.rip));
    verify_vmx(x86::vmx::vmwrite(x86::vmx::field_t::guest_rsp, registers.rsp));
    verify_vmx(x86::vmx::vmwrite(x86::vmx::field_t::guest_rflags, registers.rflags));

    registers.rsp = old_rsp;
    registers.rflags = old_rflags;

    verify(do_vm_entry_checks());

    trace_debug("Resume guest into rip=0x%x", registers.rip);
    // we must use a small asm code as to not fuckup any registers
    registers.rip = reinterpret_cast<uint64_t>(asm_vm_resume);
    asm_cpu_load_registers(&registers);
}

}

extern "C" [[noreturn]] void vm_exit_handler(hype::cpu_registers_t& registers) {
    const auto status = handle_vmexit(registers);
    trace_status("Error from VMEXIT handler!", status.error());
    catastrophic_error("failed to handle vmexit");
}
