#pragma once

#include <error.h>
#include <types.h>

#include "x86/memory.h"


namespace x86::paging {

// IA-32E Paging [SDM 3 4.5 P123]

// PDPT entry for 1GB [SDM 3 4.5 P128 "Table 4-15"]
struct huge_pdpte_t {
    union {
        struct {
            uint64_t present : 1;
            uint64_t read_write : 1;
            uint64_t user_supervisor : 1;
            uint64_t write_through : 1;
            uint64_t cache_disable : 1;
            uint64_t accessed : 1;
            uint64_t dirty : 1;
            uint64_t page_size : 1;
            uint64_t global : 1;
            uint64_t ignored0 : 3;
            uint64_t pat : 1;
            uint64_t reserved0 : 16;
            uint64_t page_address : 21;
            uint64_t ignored2 : 10;
            uint64_t protection_key : 3;
            uint64_t execute_disable : 1;
        } bits;
        uint64_t raw;
    };

    physical_address_t page_address() const noexcept;
    void page_address(physical_address_t address) noexcept;
} PACKED;

// PML4 entry [SDM 3 4.5 P127 "Table 4-14"]
struct pml4e_t {
    union {
        struct {
            uint64_t present : 1;
            uint64_t read_write : 1;
            uint64_t user_supervisor : 1;
            uint64_t write_through : 1;
            uint64_t cache_disable : 1;
            uint64_t accessed : 1;
            uint64_t ignored0 : 1;
            uint64_t must_be_zero : 1;
            uint64_t ignored1 : 4;
            uint64_t page_directory_address : 39;
            uint64_t ignored2 : 12;
            uint64_t execute_disable : 1;
        } bits;
        uint64_t raw;
    };

    physical_address_t page_directory_address() const noexcept;
    void page_directory_address(physical_address_t address) noexcept;
} PACKED;

class huge_paging_virtual_address_t {
public:
    huge_paging_virtual_address_t(const void* address) noexcept;
    huge_paging_virtual_address_t(const uintptr_t address) noexcept;

    size_t pml4e_offset() const noexcept;
    size_t pdpte_offset() const noexcept;
    size_t page_offset() const noexcept;
private:
    const uintptr_t m_address;
};


class huge_page_table_t;
common::result setup_identity_paging(huge_page_table_t& page_table) noexcept;

// supporting only a single pml4 = 512 GB
class huge_page_table_t {
public:
    static constexpr size_t PDPT_ENTRIES_COUNT = 512;

    const void* base_address() const noexcept;
    physical_address_t to_physical(const huge_paging_virtual_address_t& address) const noexcept;

    huge_pdpte_t& operator[](size_t index) noexcept;

    friend common::result x86::paging::setup_identity_paging(huge_page_table_t& page_table) noexcept;
private:
    pml4e_t m_pml4 PAGE_ALIGNED;
    huge_pdpte_t m_pdpt[PDPT_ENTRIES_COUNT] PAGE_ALIGNED;
};

bool are_huge_tables_supported() noexcept;

STATIC_ASSERT_SIZE(huge_pdpte_t, 8);
STATIC_ASSERT_SIZE(pml4e_t, 8);

namespace huge {
using virtual_address_t = huge_paging_virtual_address_t;
using page_table_t = huge_page_table_t;
} // namespace huge

}
