#pragma once

#include <error.h>


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
        };
        uint64_t raw;
    };
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
        };
        uint64_t raw;
    };
} PACKED;

const size_t PDPT_ENTRIES_COUNT = 512;

// supporting only a single pml4 = 512 GB
class huge_page_table_t {
public:
    const void* base_address() const noexcept;
    uintn_t to_physical(const void* memory) const noexcept;

    pml4e_t& pml4() noexcept;
    huge_pdpte_t& operator[](size_t index) noexcept;
private:
    pml4e_t m_pml4 PAGE_ALIGNED;
    huge_pdpte_t m_pdpt[PDPT_ENTRIES_COUNT] PAGE_ALIGNED;
};

bool are_huge_tables_supported() noexcept;

common::result setup_identity_paging(huge_page_table_t& page_table) noexcept;


static_assert(sizeof(huge_pdpte_t) == 8, "huge_pdpte_t != 8");
static_assert(sizeof(pml4e_t) == 8, "pml4e_t != 8");

}
