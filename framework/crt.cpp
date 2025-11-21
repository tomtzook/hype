
#include "heap.h"


namespace framework {

void do_abort() {
    environment::do_abort();
}

}

void* __cdecl operator new(const size_t size) {
    return operator new(size, framework::default_alignment);
}

void* __cdecl operator new(const size_t size, std::align_val_t) {
    return operator new(size);
}

void* __cdecl operator new(const size_t size, const size_t alignment) {
    auto ptr_result = framework::allocate(size, framework::memory_type_t::data, alignment);
    if (ptr_result) {
        return ptr_result.release_value();
    }

    return nullptr;
}

void* __cdecl operator new[](const size_t size) {
    return operator new(size);
}

void* __cdecl operator new[](const size_t size, std::align_val_t) {
    return operator new(size);
}

void* __cdecl operator new[](const size_t size, const size_t alignment) {
    return operator new(size, alignment);
}

void __cdecl operator delete(void* memory) {
    framework::free(memory);
}

void __cdecl operator delete(void* memory, std::align_val_t) {
    operator delete(memory);
}

void __cdecl operator delete[](void* memory) {
    operator delete(memory);
}

void __cdecl operator delete[](void* memory, std::align_val_t) {
    operator delete(memory);
}
