
#include "environment/allocation.h"

#include "crt.h"


void* __cdecl operator new(size_t size) noexcept {
    return ::operator new(size, hype::memory::alignment_t::NO_ALIGN);
}

void* __cdecl operator new(size_t size, std::align_val_t) noexcept {
    return ::operator new(size);
}

void* __cdecl operator new(size_t size, hype::memory::alignment_t alignment) noexcept {
    void* out;
    auto status = hype::environment::allocate(size, out, alignment);
    if (!status) {
        return nullptr;
    }

    return out;
}

void* __cdecl operator new[](size_t size) noexcept {
    return ::operator new(size);
}

void* __cdecl operator new[](size_t size, std::align_val_t) noexcept {
    return ::operator new(size);
}

void* __cdecl operator new[](size_t size, hype::memory::alignment_t alignment) noexcept {
    return ::operator new(size, alignment);
}

void __cdecl operator delete(void* memory) noexcept {
    hype::environment::free(memory);
}

void __cdecl operator delete(void* memory, std::align_val_t) noexcept {
    ::operator delete(memory);
}

void __cdecl operator delete[](void* memory) noexcept {
    ::operator delete(memory);
}

void __cdecl operator delete[](void* memory, std::align_val_t) noexcept {
    ::operator delete(memory);
}
