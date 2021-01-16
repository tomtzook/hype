#pragma once

#include <cstdint>

#include "macros.h"


#ifdef X86_64
typedef uint64_t uintn_t;
#else
typedef uint32_t uintn_t;
#endif
