#pragma once

#include "result.h"
#include "environment.h"

namespace framework {

result<void*> allocate(size_t size, memory_type_t type);
void free(void* ptr);

}
