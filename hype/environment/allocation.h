#pragma once

#include <common/common.h>


namespace hype::environment {

result::status allocate(size_t size,
                        void*& out,
                        memory::alignment_t alignment = memory::alignment_t::NO_ALIGN,
                        memory::type_t type = memory::type_t::DATA) noexcept;
void free(void* ptr) noexcept;

template<typename T>
T* allocate(size_t size,
            memory::alignment_t alignment = memory::alignment_t::NO_ALIGN,
            memory::type_t type = memory::type_t::DATA) noexcept {
    result::status status = {};

    void* memory;
    CHECK_AND_JUMP(status, allocate(size, memory, alignment, type));

cleanup:
    if (!status) {
        return nullptr;
    }

    return reinterpret_cast<T*>(memory);
}

}
