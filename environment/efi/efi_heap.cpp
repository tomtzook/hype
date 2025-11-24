
#include <base.h>

namespace framework::heap {

static heap g_code_heap;
static heap g_data_heap;

static heap& get_heap(const memory_type type) {
    switch (type) {
        case memory_type::code:
            return g_code_heap;
        case memory_type::data:
            return g_data_heap;
        default:
            __builtin_unreachable();
    }
}

status malloc(const memory_type type, const size_t size, void*& out_ptr) {
    auto& heap = get_heap(type);
    return heap.malloc(size, out_ptr);
}

status realloc(const memory_type type, void* ptr, const size_t new_size, void*& out_ptr) {
    auto& heap = get_heap(type);
    return heap.realloc(ptr, new_size, out_ptr);
}

status calloc(const memory_type type, const uint8_t memb, const size_t size, void*& out_ptr) {
    auto& heap = get_heap(type);
    return heap.calloc(memb, size, out_ptr);
}

status free(const memory_type type, const void* ptr) {
    auto& heap = get_heap(type);
    return heap.free(ptr);
}

}

namespace efi {

framework::status init_heap(const framework::memory_type type, void* mem, const size_t size) {
    auto& heap = framework::heap::get_heap(type);
    return heap.init(mem, size);
}

}
