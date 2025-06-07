
#include "buffer.h"

namespace hype {

buffer::buffer()
    : m_base(nullptr)
    , m_size(0)
    , m_memory_type(memory::memory_type_t::code)
{}

buffer::buffer(buffer&& other) noexcept
    : m_base(other.m_base)
    , m_size(other.m_size)
    , m_memory_type(other.m_memory_type) {
    other.m_base = nullptr;
    other.m_size = 0;
}

buffer::~buffer() {
    free();
}

buffer& buffer::operator=(buffer&& other) noexcept {
    m_base = other.m_base;
    m_size = other.m_size;
    m_memory_type = other.m_memory_type;

    other.m_base = nullptr;
    other.m_size = 0;

    return *this;
}

status_t buffer::init(const size_t size, const memory::memory_type_t memory_type) {
    void* out;
    // todo: add default alignment
    CHECK(memory::allocate(out, size, 0, memory_type));

    m_base = out;
    m_size = size;
    m_memory_type = memory_type;

    return {};
}

const void* buffer::base() const {
    return m_base;
}

size_t buffer::size() const {
    return m_size;
}

void* buffer::base() {
    return m_base;
}

void buffer::free() {
    if (m_base != nullptr) {
        memory::free(m_base);
        m_base = nullptr;
        m_size = 0;
    }
}


}
