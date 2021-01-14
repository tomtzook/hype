#pragma once

#include "types.h"


namespace x86 {

struct segment_selector_t {

    enum selector_table_t {
        TABLE_GDT = 0,
        TABLE_LDT = 1
    };

    union {
        struct {
            uint16_t request_privilege_level : 2;
            uint16_t table : 1;
            uint16_t index : 13;
        } bits;

        uint16_t raw;
    };
};

struct segment_descriptor_t {

    enum data_type_t {
        READ_ONLY = 0x0,
        READ_ONLY_ACCESSED = 0x1,
        READ_WRITE = 0x10,
        READ_WRITE_ACCESSED = 0x11,
        READ_ONLY_EXPAND_DOWN = 0x100,
        READ_ONLY_EXPAND_DOWN_ACCESSED = 0x101,
        READ_WRITE_EXPAND_DOWN = 0x110,
        READ_WRITE_EXPAND_DOWN_ACCESSED = 0x111
    };

    enum code_type_t {
        EXECUTE_ONLY = 0x1000,
        EXECUTE_ONLY_ACCESSED = 0x1001,
        EXECUTE_READ = 0x1010,
        EXECUTE_READ_ACCESSED = 0x1011,
        EXECUTE_ONLY_CONFORMING = 0x1100,
        EXECUTE_ONLY_CONFORMING_ACCESSED = 0x1101,
        EXECUTE_READ_CONFORMING = 0x1110,
        EXECUTE_READ_CONFORMING_ACCESSED = 0x1111
    };

    enum system_type_t {
        TYPE_SYSTEM_RESERVED0 = 0x0000,
        TYPE_BITS16_TSS_AVAILABLE = 0x0001,

        TYPE_LDT = 0x0010,
        TYPE_BITS16_TSS_BUSY = 0x0011,
        TYPE_BITS16_CALL_GATE = 0x0100,
        TYPE_TASK_GATE = 0x0101,
        BITS16_INTERRUPT_GATE = 0x0110,
        BITS16_TRAP_GATE = 0x0111,
        TYPE_SYSTEM_RESERVED1 = 0x1000,
        TYPE_BITS32_TSS_AVAILABLE = 0x1101,
        TYPE_SYSTEM_RESERVED2 = 0x1010,
        TYPE_BITS32_TSS_BUSY = 0x1011,
        TYPE_BITS32_CALL_GATE = 0x1100,
        TYPE_SYSTEM_RESERVED3 = 0x1101,
        TYPE_BITS32_INTERRUPT = 0x1110,
        TYPE_BITS32_TRAP_GATE = 0x1111
    };

    enum descriptor_type_t {
        SYSTEM = 0,
        CODE_OR_DATA = 1
    };

    enum default_op_size_t {
        OP_SIZE_16 = 0,
        OP_SIZE_32 = 1
    };

    enum granularity_t {
        GRANULARITY_BYTE = 0,
        GRANULARITY_PAGE = 1
    };

    union {
        struct {
            uint64_t limit_low : 16;
            uint64_t base_address_low : 16;
            uint64_t base_address_middle : 8;
            uint64_t type : 4;
            uint64_t descriptor_type : 1;
            uint64_t privilege : 2;
            uint64_t present : 1;
            uint64_t limit_high : 4;
            uint64_t available : 1;
            uint64_t long_mode : 1;
            uint64_t default_big : 1;
            uint64_t granularity : 1;
            uint64_t base_address_high : 8;
            //uint64_t base_address_upper;
            //uint64_t must_be_zero;
        } bits;

        uint64_t raw;
    };

    void* base_address() const noexcept;
    void base_address(void* value) noexcept;

    uint32_t limit() const noexcept;
    void limit(uint32_t value) noexcept;
};

struct segment_table_t {

};

struct gdtr_t {
    uint16_t limit;
    uint32_t base;

    void load() noexcept;
    void store() noexcept;
};

}
