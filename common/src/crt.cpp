#include "types.h"
#include "environment.h"

#include "crt.h"


void* operator new(size_t size) noexcept {
    void* out;
    common::result result = environment::allocate(size, &out);
    if (result) {
        return out;
    }

    return nullptr;
}

void* operator new(size_t size, environment::alignment_t alignment) noexcept {
    void* out;
    common::result result = environment::allocate(size, &out, alignment);
    if (result) {
        return out;
    }

    return nullptr;
}

void* operator new[](size_t size) noexcept {
    return ::operator new(size);
}

void* operator new[](size_t size, environment::alignment_t alignment) noexcept {
    return ::operator new(size, alignment);
}

void operator delete(void* memory) noexcept {
    environment::free(memory);
}

void operator delete[](void* memory) noexcept {
    ::operator delete(memory);
}
