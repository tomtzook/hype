#pragma once

#include <x86/intrinsics.h>

#include "debug.h"
#include "check.h"
#include "heap.h"

namespace framework {

template<memory_type_t mem>
class buffer_base {
public:
    buffer_base();
    buffer_base(const buffer_base&) = delete;
    buffer_base(buffer_base&&) noexcept;
    buffer_base(uint8_t* base, size_t size);
    ~buffer_base();

    buffer_base& operator=(const buffer_base&) = delete;
    buffer_base& operator=(buffer_base&&) noexcept;

    constexpr explicit operator bool() const;

    void reset();

    [[nodiscard]] const void* base() const;
    [[nodiscard]] void* base();
    [[nodiscard]] size_t size() const;

    template<typename t_ = uint8_t>
    const t_* data(size_t offset=0) const;
    template<typename t_ = uint8_t>
    t_* data(size_t offset=0);

    static result<buffer_base> create(size_t size);
    static result<buffer_base> copy(const buffer_base&);

private:
    [[nodiscard]] const uint8_t* _get() const {
        if (m_base == nullptr) { abort(); }
        return m_base;
    }
    [[nodiscard]] uint8_t* _get() {
        if (m_base == nullptr) { abort(); }
        return m_base;
    }

    uint8_t* m_base;
    size_t m_size;
};

template<memory_type_t mem>
buffer_base<mem>::buffer_base() : m_base(nullptr) , m_size(0) {}

template<memory_type_t mem>
buffer_base<mem>::buffer_base(buffer_base&& other) noexcept
    : m_base(other.m_base)
    , m_size(other.m_size) {
    other.m_base = nullptr;
    other.m_size = 0;
}

template<memory_type_t mem>
buffer_base<mem>::buffer_base(uint8_t* base, const size_t size) : m_base(base) , m_size(size) {}

template<memory_type_t mem>
buffer_base<mem>::~buffer_base() {
    reset();
}

template<memory_type_t mem>
buffer_base<mem>& buffer_base<mem>::operator=(buffer_base&& other) noexcept {
    m_base = other.m_base;
    m_size = other.m_size;
    other.m_base = nullptr;
    other.m_size = 0;
    return *this;
}

template<memory_type_t mem>
constexpr buffer_base<mem>::operator bool() const { return m_base != nullptr; }

template<memory_type_t mem>
void buffer_base<mem>::reset() {
    if (m_base != nullptr) {
        delete[] m_base;
        m_base = nullptr;
    }
}

template<memory_type_t mem>
const void* buffer_base<mem>::base() const { return _get(); }

template<memory_type_t mem>
void* buffer_base<mem>::base() { return _get(); }

template<memory_type_t mem>
size_t buffer_base<mem>::size() const { return _get(); }

template<memory_type_t mem>
template<typename t_>
const t_* buffer_base<mem>::data(const size_t offset) const {
    if (offset >= m_size) { abort(); }
    return reinterpret_cast<const t_*>(reinterpret_cast<uint64_t>(_get()) + offset);
}

template<memory_type_t mem>
template<typename t_>
t_* buffer_base<mem>::data(const size_t offset) {
    if (offset >= m_size) { abort(); }
    return reinterpret_cast<t_*>(reinterpret_cast<uint64_t>(_get()) + offset);
}

template<memory_type_t mem>
result<buffer_base<mem>> buffer_base<mem>::create(const size_t size) {
    // ReSharper disable once CppDFAMemoryLeak
    auto ptr = new uint8_t[size];
    verify_alloc(ptr);

    // ReSharper disable once CppDFAMemoryLeak
    return buffer_base<mem>(ptr, size);
}

template<memory_type_t mem>
result<buffer_base<mem>> buffer_base<mem>::copy(const buffer_base& other) {
    auto new_buff = verify(create(other.size()));
    memcpy(new_buff.data(), other.data(), other.size());

    return framework::move(new_buff);
}

using code_buffer = buffer_base<memory_type_t::code>;
using data_buffer = buffer_base<memory_type_t::data>;
using buffer = data_buffer;

}
