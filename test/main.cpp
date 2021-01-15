#include <iostream>

#include "x86/cpuid.h"
#include "x86/regs.h"
#include "x86/segment.h"


int main() {
    x86::cpuid_regs_t regs = {0};
    x86::cpu_id(1, regs);

    std::cout << regs.eax << std::endl;

    x86::cr0_t cr0(0);
    cr0.bits.cache_disable = true;

    x86::segment_table64_t gdt;
    gdt.store();

    printf("ADDR=%p ... LIMIT=%d\n", gdt.base_address(), gdt.limit());
    for (size_t i = 0; i < gdt.limit(); i++) {
        x86::segment_descriptor_t& descriptor = gdt[i];
        printf("(%lx) address=%p, limit=%d\n",
               i, descriptor.base_address(), descriptor.limit());
    }

    return 0;
}
