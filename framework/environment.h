#pragma once

#include "common.h"

namespace framework {

enum class memory_type_t {
    data,
    code
};

namespace environment {

[[nodiscard]] result<void*> allocate_pages(size_t pages, memory_type_t type);
void free_pages(void* ptr, size_t pages, memory_type_t type);

[[nodiscard]] physical_address_t to_physical(void* address);
[[nodiscard]] void* to_virtual(physical_address_t address);

result<> sleep(size_t microseconds);

[[noreturn]] void abort();

}
}
