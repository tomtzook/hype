#include <iostream>

#include "x64/id.h"
#include "x64/regs.h"


int main() {
    x64::general_regs_t regs = {0};
    x64::cpu_id(1, regs);

    x64::cr0_t cr0;
    cr0.store();

    std::cout << regs.eax << std::endl;
    return 0;
}
