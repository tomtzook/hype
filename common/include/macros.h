#pragma once

#include <bits/wordsize.h>

#if __WORDSIZE == 64
#define X86_64
#else
#define X86
#endif

#define PACKED __attribute__ ((packed));

#define ARRAY_SIZE(array) sizeof(array) / sizeof(array[0])

#define STATIC_ASSERT_ARRAYSIZE(array, size) static_assert(ARRAY_SIZE(array) == size,  "array != " #size)
#define STATIC_ASSERT_SIZE(type, size) static_assert(sizeof(type) == size, #type " != " #size)
#define STATIC_ASSERT_SIZE(type, size) static_assert(sizeof(type) == size, #type " != " #size)
#define STATIC_ASSERT_UPTOSIZE(type, max_size) static_assert(sizeof(type) <= (max_size), #type " > " #max_size)

#define EXTRACT_BITS(value, start_idx, mask) (((value) >> (start_idx)) & (mask))
#define CHECK_BIT(value, bit_idx) EXTRACT_BITS(value, bit_idx, 0x1)

#define REPEATARG1(arg) arg
#define REPEATARG2(arg) arg, REPEATARG1(arg)
#define REPEATARG3(arg) arg, REPEATARG2(arg)
#define REPEATARG4(arg) arg, REPEATARG3(arg)
#define REPEATARG5(arg) arg, REPEATARG4(arg)
#define REPEATARG6(arg) arg, REPEATARG5(arg)
#define REPEATARG7(arg) arg, REPEATARG6(arg)
#define REPEATARG8(arg) arg, REPEATARG7(arg)
#define REPEATARG9(arg) arg, REPEATARG8(arg)
#define REPEATARG10(arg) arg, REPEATARG9(arg)
