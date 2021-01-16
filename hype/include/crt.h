#pragma once

#include "environment.h"

_GLIBCXX_NODISCARD
void* operator new(size_t size) _GLIBCXX_USE_NOEXCEPT {
    void* out;
    hype::result result = hype::environment::allocate(size, &out);
    if (result) {
        return out;
    }

    return nullptr;
}

_GLIBCXX_NODISCARD
void* operator new(size_t size, hype::environment::alignment_t alignment) _GLIBCXX_USE_NOEXCEPT {
    void* out;
    hype::result result = hype::environment::allocate(size, &out, alignment);
    if (result) {
        return out;
    }

    return nullptr;
}

_GLIBCXX_NODISCARD
void* operator new[](size_t size) _GLIBCXX_USE_NOEXCEPT {
    return ::operator new(size);
}

_GLIBCXX_NODISCARD
void* operator new[](size_t size, hype::environment::alignment_t alignment) _GLIBCXX_USE_NOEXCEPT {
    return ::operator new(size, alignment);
}

void operator delete(void* memory) _GLIBCXX_USE_NOEXCEPT {
    hype::environment::free(memory);
}

void operator delete[](void* memory) _GLIBCXX_USE_NOEXCEPT {
    ::operator delete(memory);
}
