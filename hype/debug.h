#pragma once

#include <x86/regs.h>

#include <base.h>

namespace hype {

void hexdump(const void* data, size_t length);

void print_stack_info(uint64_t rip, uint64_t rbp);

inline __attribute__((always_inline)) void print_stack_info() {
    print_stack_info(x86::read_rip(), x86::read_rbp());
}

}
