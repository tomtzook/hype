#pragma once

#include <x86/interrupts.h>

#include <base.h>

namespace hype::interrupts {

#pragma pack(push, 1)

struct idt_t {
    static constexpr size_t descriptor_count = 256;
    static constexpr size_t ist_index = 1;

    page_aligned x86::interrupts::descriptor64_t descriptors[descriptor_count];

    struct idt_node {
        using callback = void(*)();
        callback custom_callback;
    };

    framework::array<idt_node, descriptor_count> idt_nodes;
};

#pragma pack(pop)

void trace_idt(const x86::interrupts::idtr_t& idtr);

framework::result<> setup_idt(x86::interrupts::idtr_t& idtr, idt_t& idt);
framework::result<> register_interrupt_callback(x86::interrupts::interrupt_t& interrupt, idt_t::idt_node::callback&& callback);

}
