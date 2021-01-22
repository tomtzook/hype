#pragma once

#include <types.h>
#include <debug.h>

#include "cr.h"


namespace x86 {

enum selector_table_t : uint8_t {
    TABLE_GDT = 0,
    TABLE_LDT = 1
};

enum segment_type_data_code_t : uint8_t {
    DATA_READ_ONLY = 0b0,
    DATA_READ_ONLY_ACCESSED = 0b1,
    DATA_READ_WRITE = 0b10,
    DATA_READ_WRITE_ACCESSED = 0b11,
    DATA_READ_ONLY_EXPAND_DOWN = 0b100,
    DATA_READ_ONLY_EXPAND_DOWN_ACCESSED = 0b101,
    DATA_READ_WRITE_EXPAND_DOWN = 0b110,
    DATA_READ_WRITE_EXPAND_DOWN_ACCESSED = 0b111,
    CODE_EXECUTE_ONLY = 0b1000,
    CODE_EXECUTE_ONLY_ACCESSED = 0b1001,
    CODE_EXECUTE_READ = 0b1010,
    CODE_EXECUTE_READ_ACCESSED = 0b1011,
    CODE_EXECUTE_ONLY_CONFORMING = 0b1100,
    CODE_EXECUTE_ONLY_CONFORMING_ACCESSED = 0b1101,
    CODE_EXECUTE_READ_CONFORMING = 0b1110,
    CODE_EXECUTE_READ_CONFORMING_ACCESSED = 0b1111,
};

enum segment_type_system_t {
    SYSTEM_RESERVED0 = 0b0000,
    SYSTEM_BITS16_TSS_AVAILABLE = 0b0001,
    SYSTEM_LDT = 0b0010,
    SYSTEM_BITS16_TSS_BUSY = 0b0011,
    SYSTEM_BITS16_CALL_GATE = 0b0100,
    SYSTEM_TASK_GATE = 0b0101,
    SYSTEM_BITS16_INTERRUPT_GATE = 0b0110,
    SYSTEM_BITS16_TRAP_GATE = 0b0111,
    SYSTEM_RESERVED1 = 0b1000,
    SYSTEM_BITS32_TSS_AVAILABLE = 0b1001,
    SYSTEM_RESERVED2 = 0b1010,
    SYSTEM_BITS32_TSS_BUSY = 0b1011,
    SYSTEM_BITS32_CALL_GATE = 0b1100,
    SYSTEM_RESERVED3 = 0b1101,
    SYSTEM_BITS32_INTERRUPT = 0b1110,
    SYSTEM_BITS32_TRAP_GATE = 0b1111
};

enum descriptor_type_t : uint8_t {
    DESCRIPTOR_SYSTEM = 0,
    DESCRIPTOR_CODE_OR_DATA = 1
};

enum default_op_size_t : uint8_t {
    OP_SIZE_16 = 0,
    OP_SIZE_32 = 1
};

enum granularity_t : uint8_t {
    GRANULARITY_BYTE = 0,
    GRANULARITY_PAGE = 1
};

struct segment_selector_t {
    union {
        struct {
            uint16_t request_privilege_level: 2;
            selector_table_t table: 1;
            uint16_t index: 13;
        } bits;

        uint16_t raw;
    };
} PACKED;

struct segment_descriptor_t {
    union {
        struct {
            uint64_t limit_low: 16;
            uint64_t base_address_low: 16;
            uint64_t base_address_middle: 8;
            uint8_t type: 4;
            descriptor_type_t descriptor_type: 1;
            uint64_t privilege: 2;
            bool present: 1;
            uint64_t limit_high: 4;
            bool available: 1;
            bool long_mode: 1;
            default_op_size_t default_big: 1;
            granularity_t granularity: 1;
            uint64_t base_address_high: 8;

#ifdef X86_64
            uint64_t base_address_upper: 32;
            uint64_t must_be_zero: 32;
#endif
        } bits;

        struct {
            uint64_t raw;
#ifdef X86_64
            uint64_t raw_upper;
#endif
        };
    };

    segment_descriptor_t();

    void* base_address() const noexcept;

    void base_address(void* value) noexcept;

    uint32_t limit() const noexcept;

    void limit(uint32_t value) noexcept;

    bool is_system() const noexcept;

    bool is_data() const noexcept;

    bool is_code() const noexcept;

    descriptor_type_t descriptor_type() const noexcept;

    void descriptor_type(descriptor_type_t descriptor_type) noexcept;

    default_op_size_t default_big() const noexcept;

    void default_big(default_op_size_t default_big) noexcept;

    granularity_t granularity() const noexcept;

    void granularity(granularity_t granularity) noexcept;
} PACKED;

struct segment_table_t {
    uint16_t lim;
#ifdef X86_64
    uint64_t base;
#else
    uint32_t base;
#endif

    void* base_address() const noexcept;

    uint16_t limit() const noexcept;

    const segment_descriptor_t& operator[](const segment_selector_t& selector) const noexcept;

    segment_descriptor_t& operator[](const segment_selector_t& selector) noexcept;

    const segment_descriptor_t& operator[](int index) const noexcept;

    segment_descriptor_t& operator[](int index) noexcept;
} PACKED;

void write(const segment_table_t& gdt) noexcept;
void read(segment_table_t& gdt) noexcept;

STATIC_ASSERT_SIZE(segment_selector_t, 2);

#ifdef X86_64
STATIC_ASSERT_SIZE(segment_descriptor_t, 16);
STATIC_ASSERT_SIZE(segment_table_t, 10);
#else
STATIC_ASSERT_SIZE(segment_descriptor_t, 8);
STATIC_ASSERT_SIZE(segment_table_t, 6);
#endif

}


DEBUG_NAMESPACE_START(x86);
const wchar_t* to_string(selector_table_t selector_table) noexcept;
const wchar_t* to_string(segment_type_data_code_t segment_type) noexcept;
const wchar_t* to_string(segment_type_system_t segment_type) noexcept;
const wchar_t* to_string(descriptor_type_t descriptor_type) noexcept;
const wchar_t* to_string(default_op_size_t default_op_size) noexcept;
const wchar_t* to_string(granularity_t granularity) noexcept;
const wchar_t* type_to_string(const segment_descriptor_t& descriptor) noexcept;

void trace(const segment_descriptor_t& descriptor) noexcept;
void trace(const segment_table_t& table) noexcept;
DEBUG_NAMESPACE_END
