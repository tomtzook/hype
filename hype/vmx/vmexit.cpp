
#include <x86/vmx/vmcs.h>
#include <x86/vmx/vmexit.h>
#include <x86/vmx/vmx.h>
#include <x86/cpuid.h>

#include <base.h>

#include "config.h"
#include "context.h"
#include "debug/modules.h"
#include "debug/print.h"
#include "cpu.h"
#include "vmentry.h"
#include "debug/hype_gdbstub.h"

namespace hype {

static framework::result<> handle_exception_or_nmi(cpu_registers_t&) {
    uint64_t exit_info_raw;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::vmexit_interruption_information, exit_info_raw));
    const x86::vmx::vmexit_interruption_info_t exit_info{.raw = static_cast<uint32_t>(exit_info_raw)};

    uint64_t exit_interrupt_code = 0;
    if (exit_info.bits.error_code_valid) {
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::vmexit_interruption_error_code, exit_interrupt_code));
    }

    uint64_t idt_vectoring_info_raw;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::idt_vectoring_information, idt_vectoring_info_raw));
    const x86::vmx::vmexit_interruption_info_t idt_vectoring{.raw = static_cast<uint32_t>(idt_vectoring_info_raw)};

    const auto vector = static_cast<x86::interrupts::interrupt_t>(exit_info.bits.vector);
    trace_debug("EXCEPTION[vector=%a(0x%x)]: code=0x%x(%d)",
        x86::interrupts::vector_to_str(vector), static_cast<uint16_t>(vector),
        exit_interrupt_code, exit_info.bits.error_code_valid);

    // todo: handle NMI

    if (idt_vectoring.bits.valid) {
        // todo: handle idt vector info (exception while handling another)
        return framework::err(framework::status_unsupported);
    }

    x86::vmx::vmentry_interruption_info_t inject_info{};
    inject_info.bits.vector = exit_info.bits.vector;
    inject_info.bits.type = exit_info.bits.type;
    inject_info.bits.valid = 1;

    verify_vmx(x86::vmx::vmwrite(x86::vmx::field_t::ctrl_vmentry_interruption_information_field, inject_info.raw));

    if (exit_info.bits.error_code_valid) {
        inject_info.bits.deliver_error_code = true;
        verify_vmx(x86::vmx::vmwrite(x86::vmx::field_t::ctrl_vmentry_exception_error_code, exit_interrupt_code));
    }

    if (inject_info.bits.type == x86::vmx::vmx_interrupt_type_t::software_exception ||
        inject_info.bits.type == x86::vmx::vmx_interrupt_type_t::software_interrupt ||
        inject_info.bits.type == x86::vmx::vmx_interrupt_type_t::privileged_software_exception) {
        uint64_t instruction_length;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::vmexit_instruction_length, instruction_length));
        verify_vmx(x86::vmx::vmwrite(x86::vmx::field_t::ctrl_vmentry_instruction_length, instruction_length));
    }

    if (vector == x86::interrupts::interrupt_t::page_fault) {
        // cr2 is not updated during vmexit due to page fault
        uint64_t faulting_address;
        verify_vmx(x86::vmx::vmread(x86::vmx::field_t::exit_qualification, faulting_address));
        x86::write(x86::cr2_t{faulting_address});

        if (config::print_paging_info_on_guest_page_fault) {
            // todo: works only for identity ept
            const auto guest_cr3 = verify(memory::read_current_guest_cr3());
            debug::print_page_mapping_simple(guest_cr3, faulting_address);
        }
    }

    return {};
}

static framework::result<> handle_cpuid(cpu_registers_t& registers) {
    auto cpuid = x86::cpuid(registers.eax, registers.ecx);

    switch (registers.rax) {
        case 1:
            // turn off hypervisor bit
            cpuid.ecx &= ~(1 << 31);
            break;
        default:
            break;
    }

    trace_debug("CPUID[rax=0x%x, rcx=0x%x]: eax=0x%x, ebx=0x%x, ecx=0x%x, edx=0x%x",
        registers.eax, registers.ecx, cpuid.eax, cpuid.ebx, cpuid.ecx, cpuid.edx);

    // must clear upper bits
    registers.rax = 0;
    registers.rbx = 0;
    registers.rcx = 0;
    registers.rdx = 0;

    registers.eax = cpuid.eax;
    registers.ebx = cpuid.ebx;
    registers.ecx = cpuid.ecx;
    registers.edx = cpuid.edx;

    return {};
}

static framework::result<> handle_rdmsr(cpu_registers_t& registers) {
    const auto id = registers.ecx;
    const auto value = x86::msr::read(id);

    trace_debug("RDMSR[0x%x]: value=0x%x", id, value);

    // must clear upper bits
    registers.rax = 0;
    registers.rdx = 0;

    registers.eax = static_cast<uint32_t>(value);
    registers.edx = static_cast<uint32_t>(value >> 32);

    return {};
}

static framework::result<> handle_wrmsr(const cpu_registers_t& registers) {
    const auto id = registers.ecx;
    const uint64_t low  = registers.eax;
    const uint64_t high = registers.edx;
    const uint64_t value = (high << 32) | low;

    trace_debug("WRMSR[0x%x]: value=0x%llx", id, value);

    switch (id) {
        case x86::msr::ia32_efer_t::id:
            verify_vmx(x86::vmx::vmwrite(x86::vmx::field_t::guest_efer, value));
            break;
        default:
            x86::msr::write(id, value);
    }

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

    x86::vmx::ept_pointer_t eptp;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::ctrl_ept_pointer, eptp.raw));

    // todo: works only for identity ept
    const auto guest_cr3 = verify(memory::read_current_guest_cr3());
    debug::print_page_mapping_simple(guest_cr3, linear_address);
    debug::print_ept_mapping_simple(eptp, physical_address);

    // todo: works only for identity ept and identity page table
    const auto image_info = verify(environment::get_our_image_info());
    const auto start_addr = reinterpret_cast<uint64_t>(image_info.base);
    if (physical_address >= start_addr && physical_address <= (start_addr + image_info.size)) {
        trace_debug("IN OUR IMAGE");
    }

    /*{
        trace_regs(registers);
        auto& context = get_context();
        const auto result = context.guest_memory_mapper.map(registers.rip, x86::paging::page_size);
        if (result) {
            debug::instruction_dump(result.value().data(), 4);
        }
    }*/

    return framework::err(framework::status_unsupported);
}

static framework::result<> handle_xsetbv(const cpu_registers_t& registers) {
    const auto eax = registers.eax;
    const auto edx = registers.edx;
    const auto ecx = registers.ecx;

    trace_debug("XSETBV[0x%x]: value=0x%lx", ecx, ((static_cast<uint64_t>(edx) << 32) | eax));

    __asm__ __volatile__(
        "xsetbv"
        :
        : "a"(eax), "d"(edx), "c"(ecx)
        : "memory"
    );

    return {};
}

static volatile bool aaaa = false;

framework::result<> handle_vmexit(cpu_registers_t& registers) {
    if constexpr (config::embedded_gdbstub) {
        gdbstub::start_handling_if_prompted();
    }
    if (!aaaa) {
        aaaa = true;
        //const auto image_info = verify(environment::get_our_image_info());
        //verify(memory::protect_image(get_context().ept, image_info));
    }

    const auto old_rsp = registers.rsp;
    const auto old_rflags = registers.rflags;

    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_rip, registers.rip));
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_rsp, registers.rsp));
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::guest_rflags, registers.rflags));

    uint64_t exit_reason_raw;
    verify_vmx(x86::vmx::vmread(x86::vmx::field_t::exit_reason, exit_reason_raw));

    const auto exit_reason = static_cast<x86::vmx::exit_reason_t>(exit_reason_raw & 0xffff);
    trace_debug("Exit %a (%u) From 0x%p", x86::vmx::exit_reason_str(exit_reason), static_cast<uint16_t>(exit_reason), registers.rip);

    /*if constexpr (config::decode_guest_instructions_on_vmexit || config::print_guest_memory_on_vmexit) {
        auto& context = get_context();
        const auto result = context.guest_memory_mapper.map(registers.rip, x86::paging::page_size);
        if (result) {
            if (config::decode_guest_instructions_on_vmexit) {
                debug::instruction_dump(result.value().data(), 4);
            }
            if (config::print_guest_memory_on_vmexit) {
                debug::memdump(result.value().data(), 0x10);
            }
        }
    }*/
    /*if constexpr (config::print_guest_stack_on_vmexit) {
        auto& context = get_context();
        debug::print_stack_frame(context.guest_memory_mapper, context.loaded_modules, registers.rip, registers.rbp, registers.rsp);
    }*/

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
        case x86::vmx::exit_reason_t::xsetbv:
            verify(handle_xsetbv(registers));
            break;
        default:
            trace_error("Unsupported exit %d", exit_reason);
            return framework::err(framework::status_unsupported);
    }

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

    if constexpr (config::do_vmentry_checks) {
        verify(do_vm_entry_checks());
    }

    trace_debug("Resume guest into rip=0x%llx", registers.rip);
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
