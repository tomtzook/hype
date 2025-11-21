#pragma once


// from arch
#include <x86/types.h>
#include <x86/macros.h>

// framework
#include <status.h>
#include <result.h>
#include <debug.h>
#include <verify.h>
#include <heap.h>
#include "../framework/include/memory.h"
#include <unique_ptr.h>
#include <buffer.h>

#include "x86/atomic.h"
#include "x86/vmx/vmx.h"

//__attribute__ ((aligned(4096)))
#define page_aligned alignas(0x1000)
#define b16_aligned alignas(16)

#define EXTRACT_BITS(value, start_idx, mask) (((value) >> (start_idx)) & (mask))
#define CHECK_BIT(value, bit_idx) EXTRACT_BITS(value, bit_idx, 0x1)


#define verify_vmx(...)              \
    do {                                \
        auto __status = __VA_ARGS__;    \
        if (__status != x86::vmx::instruction_result_t::success) {                \
            return framework::err(framework::status(vmx::status_category_vmx, static_cast<uint8_t>(__status)));\
        }                               \
    } while (false);

namespace vmx {

static constexpr framework::status_category_t status_category_vmx = 3;

}

namespace hype {

template<typename _t, typename _t2>
constexpr bool is_any_of(_t first, _t2 second) {
    return first == second;
}

template<typename _t, typename _t2, typename... _args>
constexpr bool is_any_of(_t first, _t2 second, _args... args) {
    return first == second || is_any_of(first, args...);
}

}
