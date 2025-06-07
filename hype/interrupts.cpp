
#include "environment.h"
#include "cpu.h"
#include "memory.h"
#include "interrupts.h"


extern "C" void* isr_stub_table[];

extern "C" void idt_handler(const uint64_t vector) {
    // todo: get error code and rip
    TRACE_ERROR("IDT Called for vector=0x%llx", vector);
    //hype::debug::deadloop();
    //hype::hlt_cpu();
    __asm__ volatile ("cli; hlt");
}

namespace hype::interrupts {

void trace_idt(const x86::interrupts::idtr_t& idtr) {
    auto table = x86::interrupts::table64_t(idtr);
    for (int i = 0; i < table.count(); i++) {
        auto& descriptor = table[i];
        TRACE_DEBUG("DESCRIPTOR: i=0x%x, address=0x%llx, selector=0x%x, dpl=0x%x, present=0x%x, ist=0x%x, type=0x%x, low=0x%llx, high=0x%llx",
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

status_t setup_idt(x86::interrupts::idtr_t& idtr, idt_t& idt) {
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
        descriptor.low.bits.ist = 0;
        descriptor.low.bits.segment_selector = selector.value;
        descriptor.low.bits.type = x86::interrupts::gate_type_t::interrupt_32;
        descriptor.address(reinterpret_cast<uint64_t>(isr_stub_table[i]));
    }

    return {};
}

}
