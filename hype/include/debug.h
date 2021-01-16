#pragma once

#include <x86/segment.h>

#include "commonefi.h"

#ifdef _DEBUG
#define DEBUG_ONLY(...) \
    do { __VA_ARGS__ } while(0)
#endif

#define TRACE_DEBUG(fmt, ...) DEBUG_ONLY( \
    Print((CHAR16*)L"" fmt "\n", ##__VA_ARGS__);  \
    )


namespace hype::debug {

void trace(const x86::segment_table_t &table);

}
