#pragma once

#include "base.h"
#include "memory.h"

namespace hype {

class buffer {
public:
    buffer();
    buffer(buffer&&) noexcept;
    buffer(buffer&) = delete;
    ~buffer();

    buffer& operator=(buffer&) = delete;
    buffer& operator=(buffer&&) noexcept;

    status_t init(size_t size, memory::memory_type_t memory_type);

    [[nodiscard]] const void* base() const;
    [[nodiscard]] size_t size() const;
    [[nodiscard]] void* base();

    template<typename T>
    const T* data(const size_t offset=0) const {
        return reinterpret_cast<const T*>(reinterpret_cast<uintptr_t>(base()) + offset);
    }
    template<typename T>
    T* data(const size_t offset=0) {
        return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(base()) + offset);
    }

private:
    void free();

    void* m_base;
    size_t m_size;
    memory::memory_type_t m_memory_type;
};

}
