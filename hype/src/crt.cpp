
#include "environment.h"
#include "crt.h"


_GLIBCXX_NODISCARD
void* operator new(size_t size) noexcept {
    void* out;
    hype::result result = hype::environment::allocate(size, &out);
    if (result) {
        return out;
    }

    return nullptr;
}

_GLIBCXX_NODISCARD
void* operator new(size_t size, hype::environment::alignment_t alignment) noexcept {
    void* out;
    hype::result result = hype::environment::allocate(size, &out, alignment);
    if (result) {
        return out;
    }

    return nullptr;
}

_GLIBCXX_NODISCARD
void* operator new[](size_t size) noexcept {
    return ::operator new(size);
}

_GLIBCXX_NODISCARD
void* operator new[](size_t size, hype::environment::alignment_t alignment) noexcept {
    return ::operator new(size, alignment);
}

void operator delete(void* memory) noexcept {
    TRACE_DEBUG("hello11 %x", memory);
    hype::environment::free(memory);
}

void operator delete[](void* memory) noexcept {
    TRACE_DEBUG("hello12 %x", memory);
    ::operator delete(memory);
}
