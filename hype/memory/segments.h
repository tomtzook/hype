#pragma once

#include <x86/segments.h>
#include <base.h>

namespace hype::memory {

#pragma pack(push, 1)

struct gdt_t {
    static constexpr size_t code_descriptor_index = 1;
    static constexpr size_t data_descriptor_index = 2;
    static constexpr size_t tss_descriptor_index = 3;

    x86::segments::descriptor_t null;
    x86::segments::descriptor_t code;
    x86::segments::descriptor_t data;
    x86::segments::descriptor64_t tr;
};

#pragma pack(pop)

void trace_gdt(const x86::segments::gdtr_t& gdtr);

// also loads the gdt
framework::result<> setup_initial_guest_gdt();
framework::result<> setup_gdt(x86::segments::gdtr_t& gdtr, gdt_t& gdt, x86::segments::tss64_t& tss);

}
