
#include "heap.h"


void* operator new(const size_t size) {
    auto ptr_result = framework::allocate(size, framework::memory_type_t::data);
    if (ptr_result) {
        return ptr_result.release_value();
    }

    return nullptr;
}

void operator delete(void* ptr) noexcept {
    framework::free(ptr);
}

void* operator new[](const size_t size) {
    return operator new(size);
}

void operator delete[](void* ptr) noexcept {
    operator delete(ptr);
}
