#pragma once

#include <x86/interrupts.h>

#include "base.h"

namespace hype::interrupts {

#pragma pack(push, 1)

struct idt_t {
    static constexpr size_t descriptor_count = 32;
    x86::interrupts::descriptor64_t descriptors[descriptor_count];
};

#pragma pack(pop)

void trace_idt(const x86::interrupts::idtr_t& idtr);

status_t setup_idt(x86::interrupts::idtr_t& idtr, idt_t& idt);

}
