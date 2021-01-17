#include "types.h"
#include "environment.h"

#include "crt.h"


_GLIBCXX_NODISCARD
void* operator new(size_t size) noexcept {
    void* out;
    common::result result = common::environment::allocate(size, &out);
    if (result) {
        return out;
    }

    return nullptr;
}

_GLIBCXX_NODISCARD
void* operator new(size_t size, common::environment::alignment_t alignment) noexcept {
    void* out;
    common::result result = common::environment::allocate(size, &out, alignment);
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
void* operator new[](size_t size, common::environment::alignment_t alignment) noexcept {
    return ::operator new(size, alignment);
}

void operator delete(void* memory) noexcept {
    common::environment::free(memory);
}

void operator delete[](void* memory) noexcept {
    ::operator delete(memory);
}
