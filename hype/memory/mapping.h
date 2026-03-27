#pragma once

#include <x86/paging/ia32e.h>
#include <base.h>
#include <vector.h>
#include <lock.h>

#include "guest.h"
#include "paging.h"

namespace hype::memory {

template<typename t_>
struct mapped_memory {
    mapped_memory(t_* mapper, void* base, size_t size);
    mapped_memory(const mapped_memory&) = delete;
    mapped_memory(mapped_memory&&) noexcept;
    ~mapped_memory();

    mapped_memory& operator=(const mapped_memory&) = delete;
    mapped_memory& operator=(mapped_memory&&) noexcept;

    [[nodiscard]] size_t size() const;

    template<typename dt_=void>
    const dt_* data(size_t offset=0) const;
    template<typename dt_=void>
    dt_* data(size_t offset=0);

    void memcpy_from(void* dst, size_t size) const;
    void memcpy_to(const void* src, size_t size);

private:
    t_* m_mapper;
    void* m_base;
    size_t m_size;
};

template<typename t_>
concept memory_mapper_type = requires(
    t_ t,
    uint64_t f1_base, size_t f1_size,
    const void* f2_base, size_t f2_size) {
    { t.map(f1_base, f1_size) } -> framework::same_as<framework::result<mapped_memory<t_>>>;
    { t.unmap(f2_base, f2_size) } -> framework::same_as<framework::result<>>;
};

template<memory_mapper_type t_>
class memory_mapper {
public:
    explicit memory_mapper(t_* mapper);

    framework::result<mapped_memory<t_>> map(uint64_t base, size_t size) const;
    framework::result<> unmap(const void* base, size_t size) const;

private:
    t_* m_mapper;
};

class guest_memory_mapper {
public:
    guest_memory_mapper(page_table_t& table, size_t pml4e_index);

    framework::result<mapped_memory<guest_memory_mapper>> map(uint64_t base, size_t size);
    framework::result<> unmap(const void* base, size_t size);

private:
    struct mapped_range {
        x86::paging::ia32e::linear_address_t mapped_base;
        size_t mapped_pages;
    };

    framework::result<x86::paging::ia32e::linear_address_t> map(size_t required_pages, const frame_ranges& ranges);

    framework::result<mapped_range*> find_and_mark_available_range(size_t page_count);
    framework::result<mapped_range*> insert_range_at(const framework::vector<mapped_range>::iterator& it, const x86::paging::ia32e::linear_address_t& base, size_t page_count);
    framework::result<mapped_range*> insert_range(const x86::paging::ia32e::linear_address_t& base, size_t page_count);
    framework::result<> remove_range(const x86::paging::ia32e::linear_address_t& base);

    size_t count_pages(const frame_ranges& ranges) const;
    size_t pages_between(const x86::paging::ia32e::linear_address_t& start, const x86::paging::ia32e::linear_address_t& end) const;

    page_table_t& m_table;
    size_t m_pml4e_index;
    framework::spin_lock m_lock;
    framework::vector<mapped_range> m_mapped; // ordered

    // an entire pml4e for mapping, so 512gb
    page_aligned x86::paging::ia32e::pdpte_t m_pdpt[x86::paging::ia32e::pdptes_in_pdpt]{};
    page_aligned x86::paging::ia32e::pde_t m_pd[x86::paging::ia32e::pdptes_in_pdpt][x86::paging::ia32e::pdes_in_directory]{};
    page_aligned x86::paging::ia32e::pte_t m_pt[x86::paging::ia32e::pdptes_in_pdpt][x86::paging::ia32e::pdes_in_directory][x86::paging::ia32e::ptes_in_table]{};
};

template<typename t_>
mapped_memory<t_>::mapped_memory(t_* mapper, void* base, const size_t size)
    : m_mapper(mapper)
    , m_base(base)
    , m_size(size)
{}

template<typename t_>
mapped_memory<t_>::mapped_memory(mapped_memory&& other) noexcept
    : m_mapper(other.m_mapper)
    , m_base(other.m_base)
    , m_size(other.m_size) {
    other.m_base = nullptr;
    other.m_size = 0;
}

template<typename t_>
mapped_memory<t_>::~mapped_memory() {
    if (m_mapper != nullptr && m_base != nullptr) {
        m_mapper->unmap(m_base, m_size);
        m_base = nullptr;
    }
}

template<typename t_>
mapped_memory<t_>& mapped_memory<t_>::operator=(mapped_memory&& other) noexcept {
    m_mapper = other.m_mapper;
    m_base = other.m_base;
    m_size = other.m_size;

    other.m_base = nullptr;
    other.m_size = 0;

    return *this;
}

template<typename t_>
size_t mapped_memory<t_>::size() const {
    return m_size;
}

template<typename t_>
template<typename dt_>
const dt_* mapped_memory<t_>::data(const size_t offset) const {
    if (m_base == nullptr) {
        catastrophic_error("mapped memory nullptr");
    }
    if (offset >= m_size) {
        catastrophic_error("out of bound of mapped memory");
    }
    return reinterpret_cast<const dt_*>(static_cast<const uint8_t*>(m_base) + offset);
}

template<typename t_>
template<typename dt_>
dt_* mapped_memory<t_>::data(const size_t offset) {
    if (m_base == nullptr) {
        catastrophic_error("mapped memory nullptr");
    }
    if (offset >= m_size) {
        catastrophic_error("out of bound of mapped memory");
    }
    return reinterpret_cast<dt_*>(static_cast<uint8_t*>(m_base) + offset);
}

template<typename t_>
void mapped_memory<t_>::memcpy_from(void* dst, const size_t size) const {
    if (m_base == nullptr) {
        catastrophic_error("mapped memory nullptr");
    }
    if (size > m_size) {
        catastrophic_error("out of bound of mapped memory");
    }
    framework::memcpy(dst, m_base, size);
}

template<typename t_>
void mapped_memory<t_>::memcpy_to(const void* src, const size_t size) {
    if (m_base == nullptr) {
        catastrophic_error("mapped memory nullptr");
    }
    if (size > m_size) {
        catastrophic_error("out of bound of mapped memory");
    }
    framework::memcpy(m_base, src, size);
}

template<memory_mapper_type t_>
memory_mapper<t_>::memory_mapper(t_* mapper)
    : m_mapper(mapper)
{}

template<memory_mapper_type t_>
framework::result<mapped_memory<t_>> memory_mapper<t_>::map(const uint64_t base, const size_t size) const {
    return m_mapper->map(base, size);
}

template<memory_mapper_type t_>
framework::result<> memory_mapper<t_>::unmap(const void* base, const size_t size) const {
    return m_mapper->unmap(base, size);
}

}
