#pragma once

#include "macros.h"


#ifdef X86_64
typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed long int64_t;
typedef int64_t intn_t;
typedef int64_t intptr_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef uint64_t uintn_t;
typedef uint64_t uintptr_t;

typedef uint64_t size_t;
#else
typedef signed char int8_t;
typedef signed short int16_t;
typedef signed long int32_t;
typedef signed long long int64_t;
typedef int32_t intn_t;
typedef int32_t intptr_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;
typedef unsigned long long uint64_t;
typedef uint32_t uintn_t;
typedef uint32_t uintptr_t;

typedef uint32_t size_t;
#endif

typedef uintn_t physical_address_t;
constexpr physical_address_t PHYSICAL_NULL_ADDRESS = 0;
