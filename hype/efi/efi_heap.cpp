
#include <base.h>

namespace framework::heap {

static heap g_code_heap;
static heap g_data_heap;

heap& get_heap(const memory_type type) {
    switch (type) {
        case memory_type::code:
            return g_code_heap;
        case memory_type::data:
            return g_data_heap;
        default:
            abort("unknown heap type");
    }
}

}
