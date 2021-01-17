#pragma once

#include "types.h"


// https://c9x.me/x86/html/file_module_x86_id_45.html
// https://wiki.osdev.org/CPUID
namespace x86 {

struct cpuid_regs_t {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
} __attribute__((packed));

struct cpuid_eax01_t {
    union {
        struct {
            uint32_t stepping_id : 4;
            uint32_t model : 4;
            uint32_t family_id : 4;
            uint32_t processor_type : 2;
            uint32_t reserved0 : 2;
            uint32_t extended_model_id : 4;
            uint32_t extended_family_id : 8;
            uint32_t reserved1 : 4;
        } bits;
        uint32_t raw;
    } eax;
    union {
        uint32_t raw;
    } ebx;
    union {
        struct {
            uint32_t sse3 : 1;
            uint32_t reserved0 : 2;
            uint32_t monitor : 1;
            uint32_t ds_cpl : 1;
            uint32_t reserved1 : 2;
            uint32_t est : 1;
            uint32_t tm2 : 1;
            uint32_t reserved2 : 1;
            uint32_t cnxt_id : 1;
            uint32_t reserved3 : 21;
        } bits;
        uint32_t raw;
    } ecx;
    union {
        struct {
            uint32_t fpu : 1;
            uint32_t vme : 1;
            uint32_t de : 1;
            uint32_t pse : 1;
            uint32_t tsc : 1;
            uint32_t pae : 1;
            uint32_t mce : 1;
            uint32_t cx8 : 1;
            uint32_t apic : 1;
            uint32_t reserved0 : 1;
            uint32_t sep : 1;
            uint32_t pge : 1;
            uint32_t mca : 1;
            uint32_t cmov : 1;
            uint32_t pat : 1;
            uint32_t pse36 : 1;
            uint32_t psn : 1;
            uint32_t clfsh : 1;
            uint32_t ds : 1;
            uint32_t acpi : 1;
            uint32_t mmx : 1;
            uint32_t fxsr : 1;
            uint32_t sse : 1;
            uint32_t see2 : 1;
            uint32_t ss : 1;
            uint32_t htt : 1;
            uint32_t tm : 1;
            uint32_t reserved1 : 1;
            uint32_t pbe : 1;
        } bits;
        uint32_t raw;
    } edx;
};

void cpu_id(unsigned int leaf, cpuid_regs_t& regs, unsigned int sub_leaf = 0) noexcept;
cpuid_regs_t cpu_id(unsigned int leaf, unsigned int sub_leaf = 0) noexcept;

void cpu_id(unsigned int leaf, void* data_struct, unsigned int sub_leaf = 0) noexcept;

template<typename cpuid_type>
void cpu_id(unsigned int leaf, cpuid_type& data_struct, unsigned int sub_leaf = 0) noexcept;
template<typename cpuid_type>
cpuid_type cpu_id(unsigned int leaf, unsigned int sub_leaf = 0) noexcept;

static_assert(sizeof(cpuid_regs_t) == 16, "cpuid_regs_t != 16");
static_assert(sizeof(cpuid_eax01_t) == 16, "cpuid_eax01_t != 16");

}

template<typename cpuid_type>
void x86::cpu_id(unsigned int leaf, cpuid_type& data_struct, unsigned int sub_leaf) noexcept {
    static_assert(sizeof(cpuid_type) == sizeof(cpuid_regs_t), "cpuid_type != cpuid regs size");
    void* data_ptr = reinterpret_cast<void*>(&data_struct);
    cpu_id(leaf, data_ptr, sub_leaf);
}

template<typename cpuid_type>
cpuid_type x86::cpu_id(unsigned int leaf, unsigned int sub_leaf) noexcept {
    static_assert(sizeof(cpuid_type) == sizeof(cpuid_regs_t), "cpuid_type != cpuid regs size");
    cpuid_type t = {0};
    cpu_id(leaf, t, sub_leaf);
    return t;
}
