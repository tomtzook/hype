#pragma once

#include "environment.h"
#include "debug.h"

_GLIBCXX_NODISCARD
void* operator new(size_t size) noexcept;
_GLIBCXX_NODISCARD
void* operator new(size_t size, hype::environment::alignment_t alignment) noexcept;

_GLIBCXX_NODISCARD
void* operator new[](size_t size) noexcept;
_GLIBCXX_NODISCARD
void* operator new[](size_t size, hype::environment::alignment_t alignment) noexcept;

void operator delete(void* memory) noexcept;
void operator delete[](void* memory) noexcept;
