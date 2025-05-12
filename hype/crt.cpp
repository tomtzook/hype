
#include "memory.h"
#include "crt.h"


void* __cdecl operator new(size_t size) {
    return ::operator new(size, 0);
}

void* __cdecl operator new(size_t size, std::align_val_t) {
    return ::operator new(size);
}

void* __cdecl operator new(size_t size, size_t alignment) {
    void* out;
    auto status = hype::memory::allocate(out, size, alignment, hype::memory::memory_type_t::data);
    if (!status) {
        return nullptr;
    }

    return out;
}

void* __cdecl operator new[](size_t size) {
    return ::operator new(size);
}

void* __cdecl operator new[](size_t size, std::align_val_t) {
    return ::operator new(size);
}

void* __cdecl operator new[](size_t size, size_t alignment) {
    return ::operator new(size, alignment);
}

void __cdecl operator delete(void* memory) {
    hype::memory::free(memory);
}

void __cdecl operator delete(void* memory, std::align_val_t) {
    ::operator delete(memory);
}

void __cdecl operator delete[](void* memory) {
    ::operator delete(memory);
}

void __cdecl operator delete[](void* memory, std::align_val_t) {
    ::operator delete(memory);
}
