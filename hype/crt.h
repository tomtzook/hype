#pragma once

#include "hype_types.h"


namespace std { // NOLINT(cert-dcl58-cpp)
enum align_val_t: size_t {};
}

void* __cdecl operator new(size_t);
void* __cdecl operator new(size_t, std::align_val_t);
void* __cdecl operator new(size_t, hype::memory::alignment_t) noexcept;

void* __cdecl operator new[](size_t);
void* __cdecl operator new[](size_t, std::align_val_t);
void* __cdecl operator new[](size_t, hype::memory::alignment_t) noexcept;

void __cdecl operator delete(void*) noexcept;
void __cdecl operator delete(void*, std::align_val_t) noexcept;
void __cdecl operator delete[](void*) noexcept;
void __cdecl operator delete[](void*, std::align_val_t) noexcept;

