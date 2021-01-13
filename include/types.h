#pragma once

#include <cstdint>

#if __WORDSIZE == 64
typedef uint64_t uintn_t;
#else
typedef uint32_t uintn_t;
#endif
