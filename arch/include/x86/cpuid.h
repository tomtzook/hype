#pragma once

#include "types.h"


namespace x86 {

struct cpuid_regs_t {
    uintn_t eax;
    uintn_t ebx;
    uintn_t ecx;
    uintn_t edx;
} __attribute__((packed));

void cpu_id(unsigned int leaf, cpuid_regs_t& regs, unsigned int sub_leaf = 0) noexcept;
void cpu_id(unsigned int leaf, void* data_struct, unsigned int sub_leaf = 0) noexcept;

template<typename T>
void cpu_id(unsigned int leaf, T& data_struct, unsigned int sub_leaf = 0) noexcept {
    void* data_ptr = reinterpret_cast<void*>(&data_struct);
    cpu_id(leaf, data_ptr, sub_leaf);
}

}
