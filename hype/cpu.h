#pragma once

#include <__stddef_offsetof.h>

#include <base.h>
#include "x86/types.h"


namespace hype {

struct b16_aligned cpu_registers_t {
    union {
        struct {
            uint8_t al;
            uint8_t ah;
        };
        uint16_t ax;
        uint32_t eax;
        uint64_t rax;
    };
    union {
        struct {
            uint8_t bl;
            uint8_t bh;
        };
        uint16_t bx;
        uint32_t ebx;
        uint64_t rbx;
    };
    union {
        struct {
            uint8_t cl;
            uint8_t ch;
        };
        uint16_t cx;
        uint32_t ecx;
        uint64_t rcx;
    };
    union {
        struct {
            uint8_t dl;
            uint8_t dh;
        };
        uint16_t dx;
        uint32_t edx;
        uint64_t rdx;
    };
    union {
        uint8_t r8b;
        uint16_t r8w;
        uint32_t r8d;
        uint64_t r8;
    };
    union {
        uint8_t r9b;
        uint16_t r9w;
        uint32_t r9d;
        uint64_t r9;
    };
    union {
        uint8_t r10b;
        uint16_t r10w;
        uint32_t r10d;
        uint64_t r10;
    };
    union {
        uint8_t r11b;
        uint16_t r11w;
        uint32_t r11d;
        uint64_t r11;
    };
    union {
        uint8_t r12b;
        uint16_t r12w;
        uint32_t r12d;
        uint64_t r12;
    };
    union {
        uint8_t r13b;
        uint16_t r13w;
        uint32_t r13d;
        uint64_t r13;
    };
    union {
        uint8_t r14b;
        uint16_t r14w;
        uint32_t r14d;
        uint64_t r14;
    };
    union {
        uint8_t r15b;
        uint16_t r15w;
        uint32_t r15d;
        uint64_t r15;
    };
    union {
        uint8_t sil;
        uint16_t si;
        uint32_t esi;
        uint64_t rsi;
    };
    union {
        uint8_t dil;
        uint16_t di;
        uint32_t edi;
        uint64_t rdi;
    };
    union {
        uint32_t eflags;
        uint64_t rflags;
    };
    union {
        uint8_t bpl;
        uint16_t bp;
        uint32_t ebp;
        uint64_t rbp;
    };
    union {
        uint8_t spl;
        uint16_t sp;
        uint32_t esp;
        uint64_t rsp;
    };
    union {
        uint16_t ip;
        uint32_t eip;
        uint64_t rip;
    };

    uint16_t cs;
    uint16_t ds;
    uint16_t es;
    uint16_t fs;
    uint16_t gs;
    uint16_t ss;
};

static_assert(offsetof(cpu_registers_t, al) == 0x0, "offsetof al");
static_assert(offsetof(cpu_registers_t, ah) == 0x1, "offsetof ah");
static_assert(offsetof(cpu_registers_t, ax) == 0x0, "offsetof ax");
static_assert(offsetof(cpu_registers_t, eax) == 0x0, "offsetof eax");
static_assert(offsetof(cpu_registers_t, rax) == 0x0, "offsetof rax");

static_assert(offsetof(cpu_registers_t, bl) == 0x8, "offsetof bl");
static_assert(offsetof(cpu_registers_t, bh) == 0x9, "offsetof bh");
static_assert(offsetof(cpu_registers_t, bx) == 0x8, "offsetof bx");
static_assert(offsetof(cpu_registers_t, ebx) == 0x8, "offsetof ebx");
static_assert(offsetof(cpu_registers_t, rbx) == 0x8, "offsetof rbx");

static_assert(offsetof(cpu_registers_t, cl) == 0x10, "offsetof cl");
static_assert(offsetof(cpu_registers_t, ch) == 0x11, "offsetof ch");
static_assert(offsetof(cpu_registers_t, cx) == 0x10, "offsetof cx");
static_assert(offsetof(cpu_registers_t, ecx) == 0x10, "offsetof ecx");
static_assert(offsetof(cpu_registers_t, rcx) == 0x10, "offsetof rcx");

static_assert(offsetof(cpu_registers_t, dl) == 0x18, "offsetof dl");
static_assert(offsetof(cpu_registers_t, dh) == 0x19, "offsetof dh");
static_assert(offsetof(cpu_registers_t, dx) == 0x18, "offsetof dx");
static_assert(offsetof(cpu_registers_t, edx) == 0x18, "offsetof edx");
static_assert(offsetof(cpu_registers_t, rdx) == 0x18, "offsetof rdx");

static_assert(offsetof(cpu_registers_t, r8b) == 0x20, "offsetof r8b");
static_assert(offsetof(cpu_registers_t, r8w) == 0x20, "offsetof r8w");
static_assert(offsetof(cpu_registers_t, r8d) == 0x20, "offsetof r8d");
static_assert(offsetof(cpu_registers_t, r8) == 0x20, "offsetof r8");

static_assert(offsetof(cpu_registers_t, r9b) == 0x28, "offsetof r9b");
static_assert(offsetof(cpu_registers_t, r9w) == 0x28, "offsetof r9w");
static_assert(offsetof(cpu_registers_t, r9d) == 0x28, "offsetof r9d");
static_assert(offsetof(cpu_registers_t, r9) == 0x28, "offsetof r9");

static_assert(offsetof(cpu_registers_t, r10b) == 0x30, "offsetof r10b");
static_assert(offsetof(cpu_registers_t, r10w) == 0x30, "offsetof r10w");
static_assert(offsetof(cpu_registers_t, r10d) == 0x30, "offsetof r10d");
static_assert(offsetof(cpu_registers_t, r10) == 0x30, "offsetof r10");

static_assert(offsetof(cpu_registers_t, r11b) == 0x38, "offsetof r11b");
static_assert(offsetof(cpu_registers_t, r11w) == 0x38, "offsetof r11w");
static_assert(offsetof(cpu_registers_t, r11d) == 0x38, "offsetof r11d");
static_assert(offsetof(cpu_registers_t, r11) == 0x38, "offsetof r11");

static_assert(offsetof(cpu_registers_t, r12b) == 0x40, "offsetof r12b");
static_assert(offsetof(cpu_registers_t, r12w) == 0x40, "offsetof r12w");
static_assert(offsetof(cpu_registers_t, r12d) == 0x40, "offsetof r12d");
static_assert(offsetof(cpu_registers_t, r12) == 0x40, "offsetof r12");

static_assert(offsetof(cpu_registers_t, r13b) == 0x48, "offsetof r13b");
static_assert(offsetof(cpu_registers_t, r13w) == 0x48, "offsetof r13w");
static_assert(offsetof(cpu_registers_t, r13d) == 0x48, "offsetof r13d");
static_assert(offsetof(cpu_registers_t, r13) == 0x48, "offsetof r13");

static_assert(offsetof(cpu_registers_t, r14b) == 0x50, "offsetof r14b");
static_assert(offsetof(cpu_registers_t, r14w) == 0x50, "offsetof r14w");
static_assert(offsetof(cpu_registers_t, r14d) == 0x50, "offsetof r14d");
static_assert(offsetof(cpu_registers_t, r14) == 0x50, "offsetof r14");

static_assert(offsetof(cpu_registers_t, r15b) == 0x58, "offsetof r15b");
static_assert(offsetof(cpu_registers_t, r15w) == 0x58, "offsetof r15w");
static_assert(offsetof(cpu_registers_t, r15d) == 0x58, "offsetof r15d");
static_assert(offsetof(cpu_registers_t, r15) == 0x58, "offsetof r15");

static_assert(offsetof(cpu_registers_t, sil) == 0x60, "offsetof sil");
static_assert(offsetof(cpu_registers_t, si) == 0x60, "offsetof si");
static_assert(offsetof(cpu_registers_t, esi) == 0x60, "offsetof esi");
static_assert(offsetof(cpu_registers_t, rsi) == 0x60, "offsetof rsi");

static_assert(offsetof(cpu_registers_t, dil) == 0x68, "offsetof dil");
static_assert(offsetof(cpu_registers_t, di) == 0x68, "offsetof di");
static_assert(offsetof(cpu_registers_t, edi) == 0x68, "offsetof edi");
static_assert(offsetof(cpu_registers_t, rdi) == 0x68, "offsetof rdi");

static_assert(offsetof(cpu_registers_t, eflags) == 0x70, "offsetof eflags");
static_assert(offsetof(cpu_registers_t, rflags) == 0x70, "offsetof rflags");

static_assert(offsetof(cpu_registers_t, bpl) == 0x78, "offsetof bpl");
static_assert(offsetof(cpu_registers_t, bp) == 0x78, "offsetof bp");
static_assert(offsetof(cpu_registers_t, ebp) == 0x78, "offsetof ebp");
static_assert(offsetof(cpu_registers_t, rbp) == 0x78, "offsetof rbp");

static_assert(offsetof(cpu_registers_t, spl) == 0x80, "offsetof spl");
static_assert(offsetof(cpu_registers_t, sp) == 0x80, "offsetof sp");
static_assert(offsetof(cpu_registers_t, esp) == 0x80, "offsetof esp");
static_assert(offsetof(cpu_registers_t, rsp) == 0x80, "offsetof rsp");

static_assert(offsetof(cpu_registers_t, ip) == 0x88, "offsetof ip");
static_assert(offsetof(cpu_registers_t, eip) == 0x88, "offsetof eip");
static_assert(offsetof(cpu_registers_t, rip) == 0x88, "offsetof rip");

static_assert(offsetof(cpu_registers_t, cs) == 0x90, "offsetof cs");
static_assert(offsetof(cpu_registers_t, ds) == 0x92, "offsetof ds");
static_assert(offsetof(cpu_registers_t, es) == 0x94, "offsetof es");
static_assert(offsetof(cpu_registers_t, fs) == 0x96, "offsetof fs");
static_assert(offsetof(cpu_registers_t, gs) == 0x98, "offsetof gs");
static_assert(offsetof(cpu_registers_t, ss) == 0x9a, "offsetof ss");

void trace_regs(const cpu_registers_t& regs);

void deadloop();
[[noreturn]] void hlt_cpu();

}

extern "C" void asm_cpu_store_registers(hype::cpu_registers_t* registers);
extern "C" [[noreturn]] void asm_cpu_load_registers(const hype::cpu_registers_t* registers);
