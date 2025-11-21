#pragma once

#include "types.h"


namespace std {
enum align_val_t: size_t {}; // NOLINT(*-dcl58-cpp)
}

namespace framework {
[[noreturn]] void do_abort();
}

void* __cdecl operator new(size_t, size_t);

inline void* __cdecl operator new(size_t, void* ptr) noexcept {
    return ptr;
}

inline void __cdecl operator delete(void*, void*) noexcept {
    // No operation needed for placement delete
}