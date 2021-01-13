#pragma once

#include "types.h"


inline uint64_t __attribute__((naked)) read_cr0()
{
    asm(R"!!(
        .intel_syntax noprefix
        mov rax, cr0
        ret
    )!!");
}

inline uint64_t __attribute__((naked)) read_cr3()
{
    asm(R"!!(
        .intel_syntax noprefix
        mov rax, cr3
        ret
    )!!");
}

inline uint64_t __attribute__((naked)) read_cr4()
{
    asm(R"!!(
        .intel_syntax noprefix
        mov rax, cr4
        ret
    )!!");
}

inline uint64_t __attribute__((naked)) write_cr0(uint64_t)
{
    asm(R"!!(
        .intel_syntax noprefix
        mov cr0, rdi
        ret
    )!!");
}

inline uint64_t __attribute__((naked)) write_cr3(uint64_t)
{
    asm(R"!!(
        .intel_syntax noprefix
        mov cr3, rdi
        ret
    )!!");
}

inline uint64_t __attribute__((naked)) write_cr4(uint64_t)
{
    asm(R"!!(
        .intel_syntax noprefix
        mov cr4, rdi
        ret
    )!!");
}

inline void __attribute__((naked)) sgdt(void *)
{
    asm(R"!!(
        .intel_syntax noprefix
        sgdt [rdi]
        ret
    )!!");
}

inline void __attribute__((naked)) sidt(void *)
{
    asm(R"!!(
        .intel_syntax noprefix
        sidt [rdi]
        ret
    )!!");
}

inline void __attribute__((naked)) lgdt(void *)
{
    asm(R"!!(
        .intel_syntax noprefix
        lgdt [rdi]
        ret
    )!!");
}

inline void __attribute__((naked)) lidt(void *)
{
    asm(R"!!(
        .intel_syntax noprefix
        lidt [rdi]
        ret
    )!!");
}
