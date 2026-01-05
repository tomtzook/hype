
#include "environment.h"
#include "cpu.h"
#include "memory.h"
#include "interrupts.h"

#include "context.h"


extern "C" void* isr_stub_table[];


static const wchar_t* interrupt_name(const x86::interrupts::interrupt_t interrupt) {
    switch (interrupt) {
        case x86::interrupts::interrupt_t::divide_error: return L"#DE";
        case x86::interrupts::interrupt_t::debug_exception: return L"#DB";
        case x86::interrupts::interrupt_t::nmi: return L"NMI";
        case x86::interrupts::interrupt_t::breakpoint: return L"#BP";
        case x86::interrupts::interrupt_t::overflow: return L"#OF";
        case x86::interrupts::interrupt_t::bound_range_exceeded: return L"#BR";
        case x86::interrupts::interrupt_t::invalid_opcode: return L"#UD";
        case x86::interrupts::interrupt_t::device_not_available: return L"#NM";
        case x86::interrupts::interrupt_t::double_fault: return L"#DF";
        case x86::interrupts::interrupt_t::coprocessor_segment_overrun: return L"CoprocessorSegmentOverrun";
        case x86::interrupts::interrupt_t::invalid_tss: return L"#TS";
        case x86::interrupts::interrupt_t::segment_not_present: return L"#NP";
        case x86::interrupts::interrupt_t::stack_segment_fault: return L"#SS";
        case x86::interrupts::interrupt_t::general_protection: return L"#GP";
        case x86::interrupts::interrupt_t::page_fault: return L"#PF";
        case x86::interrupts::interrupt_t::fpu_floating_point_error: return L"#MF";
        case x86::interrupts::interrupt_t::alignment_check: return L"#AC";
        case x86::interrupts::interrupt_t::machine_check: return L"#MC";
        case x86::interrupts::interrupt_t::simd_floating_point_exception: return L"#XM";
        case x86::interrupts::interrupt_t::virtualization_exception: return L"#VE";
        default: return L"N/A";
    }
}

static void handle_general_protection(const uint64_t error_code) {
    if (error_code == 0) {
        trace_debug("General Protection fault. Error code is 0");
        return;
    }

    x86::interrupts::selector_error_code_t selector{};
    selector.raw = error_code;

    switch (selector.bits.tbl) {
        case x86::interrupts::selector_error_code_table_t::gdt: {
            auto gdt = x86::segments::table_t(x86::read<x86::segments::gdtr_t>());
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

    const x86::paging::ia32e::linear_address_t fault_address{address};
    if (fault_address.small.pml4e == hype::memory::page_table_t::stack_guard_pml4e) {
        trace_debug("Fault occurred withing stack guard, likely stack overflow");
    }
}

extern "C" void idt_handler(const uint64_t vector, const uint64_t error_code, const uint64_t rip, const uint16_t cs) {
    const auto interrupt = static_cast<x86::interrupts::interrupt_t>(vector);
    trace_debug("IDT Called from 0x%p for [0x%x] %S(0x%x) cs=0x%x", rip, vector, interrupt_name(interrupt), error_code, cs);

    switch (interrupt) {
        case x86::interrupts::interrupt_t::general_protection:
            handle_general_protection(error_code);
            break;
        case x86::interrupts::interrupt_t::page_fault:
            handle_page_fault(error_code, rip, x86::read<x86::cr2_t>().raw);
            break;
        default:
            break;
    }

    if (vector < 32) {
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

    idtr.base_address = environment::to_physical(&idt);
    idtr.limit = sizeof(idt) - 1;

    x86::segments::selector_t selector{};
    selector.bits.table = x86::segments::table_type_t::gdt;
    selector.bits.rpl = 0;
    selector.bits.index = memory::gdt_t::code_descriptor_index;

    for (int i = 0; i < idt_t::descriptor_count; ++i) {
        auto& descriptor = idt.descriptors[i];
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
