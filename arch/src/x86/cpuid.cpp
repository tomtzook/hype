#include "x86/intrinsic.h"
#include "x86/cpuid.h"


void x86::cpu_id(unsigned int leaf, cpuid_regs_t& regs, unsigned int sub_leaf) noexcept {
    cpu_id(leaf, &regs, sub_leaf);
}

x86::cpuid_regs_t x86::cpu_id(unsigned int leaf, unsigned int sub_leaf) noexcept {
    cpuid_regs_t regs = {0};
    cpu_id(leaf, regs, sub_leaf);
    return regs;
}

void x86::cpu_id(unsigned int leaf, void* data_struct, unsigned int sub_leaf) noexcept {
    _cpuid(leaf, sub_leaf, reinterpret_cast<uint32_t*>(data_struct));
}
