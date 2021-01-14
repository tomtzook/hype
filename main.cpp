#include <iostream>

#include "x86/cpuid.h"
#include "x86/regs.h"
#include "x86/segment.h"


int main() {
    x86::cpuid_regs_t regs = {0};
    x86::cpu_id(1, regs);

    x86::cr0_t cr0;
    cr0.bits.cache_disable = true;
    cr0.store();

    x86::segment_descriptor_t sd;
    sd.bits.default_big = x86::segment_descriptor_t::default_op_size_t::OP_SIZE_16;

    std::cout << regs.eax << std::endl;
    return 0;
}
