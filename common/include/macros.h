#pragma once

#include <bits/wordsize.h>

#if __WORDSIZE == 64
#define X86_64
#else
#define X86
#endif

#define PACKED __attribute__ ((packed));


#define EXTRACT_BITS(value, start_idx, mask) (((value) >> (start_idx)) & (mask))
#define CHECK_BIT(value, bit_idx) EXTRACT_BITS(value, bit_idx, 0x1)