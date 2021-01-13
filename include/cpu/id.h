#pragma once

#include "regs.h"

namespace cpu {

void cpu_id(unsigned int leaf, general_regs_t& regs, unsigned int sub_leaf = 0);
void cpu_id(unsigned int leaf, void* data_struct, unsigned int sub_leaf = 0);

template<typename T>
void cpu_id(unsigned int leaf, T& data_struct, unsigned int sub_leaf = 0) {
    void* data_ptr = reinterpret_cast<void*>(&data_struct);
    cpu_id(leaf, data_ptr, sub_leaf);
}


} // namespace cpu
