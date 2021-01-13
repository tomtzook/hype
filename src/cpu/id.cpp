#include <cpuid.h>
#include <cstring>

#include "cpu/id.h"


void cpu::cpu_id(unsigned int leaf, general_regs_t& regs, unsigned int sub_leaf) {
    if (0 == sub_leaf) {
        // returns 1 for success, 0 for error
        __get_cpuid(leaf,
                    reinterpret_cast<unsigned int*>(&regs.eax),
                    reinterpret_cast<unsigned int*>(&regs.ebx),
                    reinterpret_cast<unsigned int*>(&regs.ecx),
                    reinterpret_cast<unsigned int*>(&regs.edx));
    } else {
        __get_cpuid_count(leaf, sub_leaf,
                          reinterpret_cast<unsigned int*>(&regs.eax),
                          reinterpret_cast<unsigned int*>(&regs.ebx),
                          reinterpret_cast<unsigned int*>(&regs.ecx),
                          reinterpret_cast<unsigned int*>(&regs.edx));
    }
}

void cpu::cpu_id(unsigned int leaf, void* data_struct, unsigned int sub_leaf) {
    general_regs_t regs = {0};
    cpu_id(leaf, regs, sub_leaf);

    memcpy(data_struct, &regs, sizeof(general_regs_t));
}
