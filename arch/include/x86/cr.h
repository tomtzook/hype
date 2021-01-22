#pragma once

#include <types.h>

namespace x86 {

struct cr0_t {
    union {
        struct {
            uint64_t protection_enable : 1;
#define CR0_PE_MASK (1 << 0)
            uint64_t monitor_coprocessor : 1;
#define CR0_MP_MASK (1 << 1)
            uint64_t emulate_fpu : 1;
#define CR0_EM_MASK (1 << 2)
            uint64_t task_switched : 1;
#define CR0_TS_MASK (1 << 3)
            uint64_t extension_type : 1;
#define CR0_ET_MASK (1 << 4)
            uint64_t numeric_error : 1;
#define CR0_NE_MASK (1 << 5)
            uint64_t reserved1 : 10;
            uint64_t write_protect : 1;
#define CR0_WP_MASK (1 << 16)
            uint64_t reserved2 : 1;
            uint64_t alignment_mask : 1;
#define CR0_AM_MASK (1 << 18)
            uint64_t reserved3 : 10;
            uint64_t not_write_through : 1;
#define CR0_NW_MASK (1 << 29)
            uint64_t cache_disable : 1;
#define CR0_CD_MASK (1 << 30)
            uint64_t paging_enable : 1;
#define CR0_PG_MASK (1 << 31)
        } bits;

        uint64_t raw;
    };

    cr0_t() noexcept;
    cr0_t(uint64_t raw) noexcept;
} PACKED;

struct cr3_t {
    union {
        struct {
            uint64_t ignored0 : 3;
            uint64_t oage_write_through : 1;
            uint64_t page_cache_disable : 1;
            uint64_t ignored1 : 7;
            uint64_t page_table_address : 52;
        } bits;

        uint64_t raw;
    };

    cr3_t() noexcept;
    cr3_t(uint64_t raw) noexcept;
} PACKED;

struct cr4_t {
    union {
        struct {
            uint64_t virtual_mode_extensions : 1;
            uint64_t protected_mode_virtual_interrupts : 1;
            uint64_t timestamp_disable : 1;
            uint64_t debugging_extensions : 1;
            uint64_t page_size_extensions : 1;
            uint64_t physical_address_extension : 1;
            uint64_t machine_check_enable : 1;
            uint64_t page_global_enable : 1;
            uint64_t performance_monitoring_counter_enable : 1;
            uint64_t os_fxsave_fxrstor_support : 1;
            uint64_t os_xmm_exception_support : 1;
            uint64_t usermode_instruction_prevention : 1;
            uint64_t reserved1 : 1;
            uint64_t vmx_enable : 1;
            uint64_t smx_enable : 1;
            uint64_t reserved2 : 1;
            uint64_t fsgsbase_enable : 1;
            uint64_t pcid_enable : 1;
            uint64_t os_xsave : 1;
            uint64_t reserved3 : 1;
            uint64_t smep_enable : 1;
            uint64_t smap_enable : 1;
            uint64_t protection_key_enable : 1;
        } bits;

        uint64_t raw;
    };

    cr4_t() noexcept;
    cr4_t(uint64_t raw) noexcept;
} PACKED;

void read(cr0_t& reg) noexcept;
void write(const cr0_t& reg) noexcept;

void read(cr3_t& reg) noexcept;
void write(const cr3_t &reg) noexcept;

void read(cr4_t& reg) noexcept;
void write(const cr4_t &reg) noexcept;

}
