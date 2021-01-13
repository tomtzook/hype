#pragma once

#include "types.h"

namespace cpu {

struct general_regs_t {
    uintn_t eax;
    uintn_t ebx;
    uintn_t ecx;
    uintn_t edx;
};

} // namespace cpu
