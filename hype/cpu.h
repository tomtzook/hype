#pragma once
#include "x86/types.h"


namespace hype {

struct cpu_registers_t {
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rflags;
    uint64_t rbp;
    uint64_t rsp;
    uint64_t rip;

    uint16_t cs;
    uint16_t ds;
    uint16_t es;
    uint16_t fs;
    uint16_t gs;
    uint16_t ss;
};


void read_registers(cpu_registers_t& registers);
void write_registers(const cpu_registers_t& registers);

}
