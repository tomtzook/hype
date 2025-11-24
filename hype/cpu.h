#pragma once

#include <__stddef_offsetof.h>

#include <base.h>
#include "x86/types.h"


namespace hype {

struct b16_aligned cpu_registers_t {
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

static_assert(offsetof(cpu_registers_t, rax) == 0x0, "offsetof rax");
static_assert(offsetof(cpu_registers_t, rbx) == 0x8, "offsetof rbx");
static_assert(offsetof(cpu_registers_t, rcx) == 0x10, "offsetof rcx");
static_assert(offsetof(cpu_registers_t, rdx) == 0x18, "offsetof rdx");
static_assert(offsetof(cpu_registers_t, r8) == 0x20, "offsetof r8");
static_assert(offsetof(cpu_registers_t, r9) == 0x28, "offsetof r9");
static_assert(offsetof(cpu_registers_t, r10) == 0x30, "offsetof r10");
static_assert(offsetof(cpu_registers_t, r11) == 0x38, "offsetof r11");
static_assert(offsetof(cpu_registers_t, r12) == 0x40, "offsetof r12");
static_assert(offsetof(cpu_registers_t, r13) == 0x48, "offsetof r13");
static_assert(offsetof(cpu_registers_t, r14) == 0x50, "offsetof r14");
static_assert(offsetof(cpu_registers_t, r15) == 0x58, "offsetof r15");
static_assert(offsetof(cpu_registers_t, rsi) == 0x60, "offsetof rsi");
static_assert(offsetof(cpu_registers_t, rdi) == 0x68, "offsetof rdi");
static_assert(offsetof(cpu_registers_t, rflags) == 0x70, "offsetof rflags");
static_assert(offsetof(cpu_registers_t, rbp) == 0x78, "offsetof rbp");
static_assert(offsetof(cpu_registers_t, rsp) == 0x80, "offsetof rsp");
static_assert(offsetof(cpu_registers_t, rip) == 0x88, "offsetof rip");
static_assert(offsetof(cpu_registers_t, cs) == 0x90, "offsetof cs");
static_assert(offsetof(cpu_registers_t, ds) == 0x92, "offsetof ds");
static_assert(offsetof(cpu_registers_t, es) == 0x94, "offsetof es");
static_assert(offsetof(cpu_registers_t, fs) == 0x96, "offsetof fs");
static_assert(offsetof(cpu_registers_t, gs) == 0x98, "offsetof gs");
static_assert(offsetof(cpu_registers_t, ss) == 0x9a, "offsetof ss");

void deadloop();
[[noreturn]] void hlt_cpu();

}

extern "C" void asm_cpu_store_registers(hype::cpu_registers_t* registers);
extern "C" [[noreturn]] void asm_cpu_load_registers(const hype::cpu_registers_t* registers);
