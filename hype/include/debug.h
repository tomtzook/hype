#pragma once

#include <x86/segment.h>

#include "commonefi.h"

#ifdef _DEBUG
#define DEBUG_ONLY(...) \
    do { __VA_ARGS__ } while(0)
#else
#define DEBUG_ONLY(...)
#endif

#define TRACE_DEBUG(fmt, ...) DEBUG_ONLY( \
    Print((CHAR16*)L"[DEBUG] " fmt "\n", ##__VA_ARGS__);  \
    )

#define TRACE_ERROR(fmt, ...) DEBUG_ONLY( \
    Print((CHAR16*)L"[ERROR] " fmt "\n", ##__VA_ARGS__);  \
    )

#define TRACE_CPUID_REG(leaf, subleaf, reg, value) \
    TRACE_DEBUG("CPUID[%d:%d].%s = 0x%x", leaf, subleaf, L"" reg, value)

#define TRACE_CPUID_BIT(leaf, subleaf, reg, bit, value) \
    TRACE_DEBUG("CPUID[%d:%d].%s.%s = 0x%x", leaf, subleaf, L"" reg, L"" bit, value)

#define TRACE_REG_BIT(reg, bit, value) \
    TRACE_DEBUG("%s.%s = 0x%x", L"" reg, L"" bit, value);

#ifdef _DEBUG
namespace hype::debug {

void deadloop() noexcept;

void trace(const x86::segment_table_t &table) noexcept;

}
#endif
