#include <x86/paging/paging.h>
#include <heap.h>

#include "base.h"

namespace framework {

#pragma pack(push, 1)

struct allocation_header_t {
    uintptr_t ptr;
    size_t pages;
    memory_type_t type;
};

#pragma pack(pop)

result<void*> allocate(const size_t size, const memory_type_t type, const size_t alignment) {
    const size_t pages = (size + sizeof(allocation_header_t) + alignment - 1) / x86::paging::page_size;
    // todo: allocate/free pages will not be available after exitbootservices, need to replace it if wanted by our own heap
    void* memory = verify(framework::environment::allocate_pages(pages, type));

    void* aligned_mem;
    if (0 != alignment) {
        aligned_mem = reinterpret_cast<void*>((reinterpret_cast<uintptr_t>(memory) +
                sizeof(allocation_header_t)  + alignment - 1) & ~(uintptr_t)(alignment - 1));
    } else {
        aligned_mem = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(memory) + sizeof(allocation_header_t));
    }

    const auto header = reinterpret_cast<allocation_header_t*>(reinterpret_cast<uintptr_t>(aligned_mem) - sizeof(allocation_header_t));
    header->ptr = reinterpret_cast<uintptr_t>(memory);
    header->pages = pages;
    header->type = type;

    return framework::ok(aligned_mem);
}

void free(void* ptr) {
    const auto header = reinterpret_cast<allocation_header_t*>(reinterpret_cast<uintptr_t>(ptr) - sizeof(allocation_header_t));
    environment::free_pages(reinterpret_cast<void*>(header->ptr), header->pages, header->type);
}

}
