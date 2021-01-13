#include <iostream>

#include "cpu/id.h"


int main() {
    cpu::general_regs_t regs = {0};
    cpu::cpu_id(1, regs);

    std::cout << regs.eax << std::endl;
    return 0;
}
