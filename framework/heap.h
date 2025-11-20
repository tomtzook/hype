#pragma once

#include "result.h"
#include "environment.h"

namespace framework {

static constexpr size_t default_alignment = 16;

result<void*> allocate(size_t size, memory_type_t type = memory_type_t::data, size_t alignment = default_alignment);
void free(void* ptr);

}
