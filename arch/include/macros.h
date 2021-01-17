#pragma once

#include <cstdint>

#if __WORDSIZE == 64
#define X86_64
#else
#define X86
#endif

#define PACKED __attribute__ ((packed));
