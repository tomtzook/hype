#pragma once

#include "types.h"

namespace x86 {

struct cr0_t {
    union {
        struct {
            uint64_t protection_enable : 1;
            uint64_t monitor_coprocessor : 1;
            uint64_t emulate_fpu : 1;
            uint64_t task_switched : 1;
            uint64_t extension_type : 1;
            uint64_t numeric_error : 1;
            uint64_t reserved1 : 10;
            uint64_t write_protect : 1;
            uint64_t reserved2 : 1;
            uint64_t alignment_mask : 1;
            uint64_t reserved3 : 10;
            uint64_t not_write_through : 1;
            uint64_t cache_disable : 1;
            uint64_t paging_enable : 1;
        } bits;

        uint64_t raw;
    };

    cr0_t() noexcept;
    explicit cr0_t(uint64_t raw) noexcept;

    void clear() noexcept;
    void load() noexcept;
    void store() noexcept;
};

struct cr3_t {
    union {
        struct {
            uint64_t pcid : 12;
            uint64_t page_frame_number : 36;
            uint64_t reserved1 : 12;
            uint64_t reserved2 : 3;
            uint64_t pcid_invalidate : 1;
        } bits;

        uint64_t raw;
    };

    cr3_t() noexcept;
    explicit cr3_t(uint64_t raw) noexcept;

    void clear() noexcept;
    void load() noexcept;
    void store() noexcept;
};

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
    explicit cr4_t(uint64_t raw) noexcept;

    void clear() noexcept;
    void load() noexcept;
    void store() noexcept;
};

}
