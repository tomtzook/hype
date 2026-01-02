
#include <lock.h>
#include <base.h>


static framework::heap::heap g_code_heap;
static framework::heap::heap g_data_heap;
static framework::spin_lock g_lock;

namespace framework::heap {

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
    unique_lock lock(g_lock);

    auto& heap = get_heap(type);
    return heap.malloc(size, out_ptr);
}

status realloc(const memory_type type, void* ptr, const size_t new_size, void*& out_ptr) {
    unique_lock lock(g_lock);

    auto& heap = get_heap(type);
    return heap.realloc(ptr, new_size, out_ptr);
}

status calloc(const memory_type type, const uint8_t memb, const size_t size, void*& out_ptr) {
    unique_lock lock(g_lock);

    auto& heap = get_heap(type);
    return heap.calloc(memb, size, out_ptr);
}

status free(const memory_type type, const void* ptr) {
    unique_lock lock(g_lock);

    auto& heap = get_heap(type);
    return heap.free(ptr);
}

}

namespace efi {

framework::status init_heap(const framework::memory_type type, void* mem, const size_t size) {
    framework::unique_lock lock(g_lock);

    auto& heap = framework::heap::get_heap(type);
    return heap.init(mem, size);
}

}
