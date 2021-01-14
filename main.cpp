#include <iostream>

#include "x86/cpuid.h"
#include "x86/regs.h"


int main() {
    x86::cpuid_regs_t regs = {0};
    x86::cpu_id(1, regs);

    x86::cr0_t cr0;
    cr0.store();

    std::cout << regs.eax << std::endl;
    return 0;
}
