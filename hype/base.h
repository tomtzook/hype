#pragma once


// from arch
#include "x86/types.h"
#include "x86/macros.h"

// ours
#include "crt.h"
#include "debug.h"
#include "status.h"

#define page_aligned __attribute__ ((aligned(4096)))

#define EXTRACT_BITS(value, start_idx, mask) (((value) >> (start_idx)) & (mask))
#define CHECK_BIT(value, bit_idx) EXTRACT_BITS(value, bit_idx, 0x1)
