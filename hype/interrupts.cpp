
#include <x86/dr.h>
#include <x86/rflags.h>

#include "environment.h"
#include "cpu.h"
#include "interrupts.h"

#include "context.h"
#include "debug/hype_gdbstub.h"


extern "C" void* isr_stub_table[];

static void handle_general_protection(const uint64_t error_code) {
    if (error_code == 0) {
        trace_debug("General Protection fault. Error code is 0");
        return;
    }

    x86::interrupts::selector_error_code_t selector{};
    selector.raw = error_code;

    switch (selector.bits.tbl) {
        case x86::interrupts::selector_error_code_table_t::gdt: {
            const auto gdtr = x86::read<x86::segments::gdtr_t>();
            auto gdt = x86::segments::table_t(gdtr);
            if (gdt.count() < selector.bits.index) {
                const auto& segment = gdt[selector.bits.index];
                trace_debug("General Protection fault. At: 0x%x GDT segment, base=0x%llx, limit=0x%llx, s=0x%x, type=0x%x, avail=0x%x, present=0x%x, db=0x%x, dpl=0x%x, long=0x%x, gran=0x%x, raw=0x%llx",
                    selector.bits.index,
                    segment.base_address(), segment.limit(),
                    segment.bits.s,
                    segment.bits.type,
                    segment.bits.available,
                    segment.bits.present,
                    segment.bits.default_db,
                    segment.bits.dpl,
                    segment.bits.long_mode,
                    segment.bits.granularity,
                    segment.raw);
            } else {
                trace_debug("General Protection fault. At 0x%x GDT segment (not in gdt)", selector.bits.index);
                //hype::memory::trace_gdt(gdtr);
            }
            break;
        }
        case x86::interrupts::selector_error_code_table_t::idt1:
        case x86::interrupts::selector_error_code_table_t::idt2: {
            auto idt = x86::interrupts::table64_t(x86::read<x86::interrupts::idtr_t>());
            if (idt.count() < selector.bits.index) {
                const auto& descriptor = idt[selector.bits.index];
                trace_debug("General Protection fault. At: 0x%x IDT descriptor, address=0x%llx, selector=0x%x, dpl=0x%x, present=0x%x, ist=0x%x, type=0x%x, low=0x%llx, high=0x%llx",
                      selector.bits.index,
                      descriptor.address(),
                      descriptor.low.bits.segment_selector,
                      descriptor.low.bits.dpl,
                      descriptor.low.bits.present,
                      descriptor.low.bits.ist,
                      static_cast<uint16_t>(descriptor.low.bits.type),
                      descriptor.low.raw,
                      descriptor.high.raw);
            } else {
                trace_debug("General Protection fault. At 0x%x IDT descriptor (not in idt)", selector.bits.index);
            }
            break;
        }
        case x86::interrupts::selector_error_code_table_t::ldt:
            trace_debug("General Protection fault. At 0x%x LDT segment", selector.bits.index);
            break;
    }
}

static void handle_page_fault(const uint64_t error_code, const uint64_t rip, const uint64_t address) {
    trace_debug("Page fault at 0x%p accessing 0x%p", rip, address);

    const x86::interrupts::page_fault_error_code_t code{.raw = static_cast<uint32_t>(error_code)};
    if (code.bits.p) {
        if (code.bits.w) {
            trace_debug("Page is not writable");
        } else if (code.bits.i) {
            trace_debug("Page is not executable");
        } else {
            trace_debug("Page is not readable");
        }
    } else {
        trace_debug("Page is not present");
    }

    const x86::paging::ia32e::linear_address_t fault_address{address};
    if (fault_address.small.pml4e == hype::memory::page_table_t::stack_guard_pml4e) {
        trace_debug("Fault occurred within stack guard, likely stack overflow");
    }
}

bool handle_debug_break() {
    auto dr6 = x86::read<x86::dr6_t>();
    const auto dr7 = x86::read<x86::dr7_t>();

    bool did_breakpoint_hit = false;
    if (dr6.bits.bp_0_cond) {
        const auto dr_addr = x86::read<x86::dr0_t>();
        trace_debug("Debug Break on bp0: addr=0x%x, len=%d, cond=%d", dr_addr.raw, dr7.bits.length_bp_0, dr7.bits.condition_bp_0);

        dr6.bits.bp_0_cond = false;
        did_breakpoint_hit = true;
    } else if (dr6.bits.bp_1_cond) {
        const auto dr_addr = x86::read<x86::dr1_t>();
        trace_debug("Debug Break on bp1: addr=0x%x, len=%d, cond=%d", dr_addr.raw, dr7.bits.length_bp_1, dr7.bits.condition_bp_1);

        dr6.bits.bp_1_cond = false;
        did_breakpoint_hit = true;
    } else if (dr6.bits.bp_2_cond) {
        const auto dr_addr = x86::read<x86::dr2_t>();
        trace_debug("Debug Break on bp2: addr=0x%x, len=%d, cond=%d", dr_addr.raw, dr7.bits.length_bp_2, dr7.bits.condition_bp_2);

        dr6.bits.bp_2_cond = false;
        did_breakpoint_hit = true;
    } else if (dr6.bits.bp_3_cond) {
        const auto dr_addr = x86::read<x86::dr3_t>();
        trace_debug("Debug Break on bp3: addr=0x%x, len=%d, cond=%d", dr_addr.raw, dr7.bits.length_bp_3, dr7.bits.condition_bp_3);

        dr6.bits.bp_3_cond = false;
        did_breakpoint_hit = true;
    }

    if (did_breakpoint_hit) {
        x86::write(dr6);
        return true;
    }

    return false;
}

extern "C" void idt_handler(const uint64_t vector, const uint64_t error_code, const uint64_t rip, const uint16_t cs) {
    const auto interrupt = static_cast<x86::interrupts::interrupt_t>(vector);

    auto& cpu = hype::get_current_vcpu();
    auto* registers = reinterpret_cast<hype::cpu_registers_t*>(reinterpret_cast<uint64_t>(cpu.interrupt_stack.end()) - hype::vcpu_t::stack_shadow_space);

    trace_debug("IDT Called from 0x%p for [0x%x] %a(0x%x) cs=0x%x", rip, vector, x86::interrupts::vector_to_str(interrupt), error_code, cs);
    hype::trace_regs(*registers);

    bool did_breakpoint_hit = false;
    switch (interrupt) {
        case x86::interrupts::interrupt_t::general_protection:
            handle_general_protection(error_code);
            break;
        case x86::interrupts::interrupt_t::page_fault:
            handle_page_fault(error_code, rip, x86::read<x86::cr2_t>().raw);
            break;
        case x86::interrupts::interrupt_t::debug_exception:
            did_breakpoint_hit = handle_debug_break();
            break;
        default:
            break;
    }

    // todo: multiprocessing, state per processor for gdb
    // todo: lock other processors???
    gdbstub::handle_interrupt(interrupt, *registers);

    if (did_breakpoint_hit) {
        // breakpoint hit, we must set the resume flag
        x86::rflags_t rflags{registers->rflags};
        rflags.bits.resume_flag = true;
        registers->rflags = rflags.raw;
    }

    if (vector < 32 && vector != 3) {
        // these are problem interrupts, halt
        hype::hlt_cpu();
    }
}

namespace hype::interrupts {

void trace_idt(const x86::interrupts::idtr_t& idtr) {
    auto table = x86::interrupts::table64_t(idtr);
    for (int i = 0; i < table.count(); i++) {
        auto& descriptor = table[i];
        trace_debug("DESCRIPTOR: i=0x%x, address=0x%llx, selector=0x%x, dpl=0x%x, present=0x%x, ist=0x%x, type=0x%x, low=0x%llx, high=0x%llx",
                    i,
                    descriptor.address(),
                    descriptor.low.bits.segment_selector,
                    descriptor.low.bits.dpl,
                    descriptor.low.bits.present,
                    descriptor.low.bits.ist,
                    static_cast<uint16_t>(descriptor.low.bits.type),
                    descriptor.low.raw,
                    descriptor.high.raw);
    }
}

framework::result<> setup_idt(x86::interrupts::idtr_t& idtr, idt_t& idt) {
    memset(&idt, 0, sizeof(idt));

    idtr.base_address = environment::to_physical(idt.descriptors);
    idtr.limit = sizeof(idt.descriptors) - 1;

    x86::segments::selector_t selector{};
    selector.bits.table = x86::segments::table_type_t::gdt;
    selector.bits.rpl = 0;
    selector.bits.index = memory::gdt_t::code_descriptor_index;

    for (int i = 0; i < idt_t::descriptor_count; ++i) {
        auto& descriptor = idt.descriptors[i];
        descriptor.low.raw = 0;
        descriptor.high.raw = 0;
        descriptor.low.bits.dpl = 0;
        descriptor.low.bits.present = 1;
        descriptor.low.bits.ist = idt_t::ist_index;
        descriptor.low.bits.segment_selector = selector.value;
        descriptor.low.bits.type = x86::interrupts::gate_type_t::interrupt_32;
        descriptor.address(reinterpret_cast<uint64_t>(isr_stub_table[i]));
    }

    return {};
}

}
