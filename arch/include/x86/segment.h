#pragma once

#include "types.h"
#include "cr.h"


namespace x86 {

enum selector_table_t {
    TABLE_GDT = 0,
    TABLE_LDT = 1
};

enum data_type_t {
    READ_ONLY = 0b0,
    READ_ONLY_ACCESSED = 0b1,
    READ_WRITE = 0b10,
    READ_WRITE_ACCESSED = 0b11,
    READ_ONLY_EXPAND_DOWN = 0b100,
    READ_ONLY_EXPAND_DOWN_ACCESSED = 0b101,
    READ_WRITE_EXPAND_DOWN = 0b110,
    READ_WRITE_EXPAND_DOWN_ACCESSED = 0b111
};

enum code_type_t {
    EXECUTE_ONLY = 0b1000,
    EXECUTE_ONLY_ACCESSED = 0b1001,
    EXECUTE_READ = 0b1010,
    EXECUTE_READ_ACCESSED = 0b1011,
    EXECUTE_ONLY_CONFORMING = 0b1100,
    EXECUTE_ONLY_CONFORMING_ACCESSED = 0b1101,
    EXECUTE_READ_CONFORMING = 0b1110,
    EXECUTE_READ_CONFORMING_ACCESSED = 0b1111
};

enum system_type_t {
    TYPE_SYSTEM_RESERVED0 = 0b0000,
    TYPE_BITS16_TSS_AVAILABLE = 0b0001,
    TYPE_LDT = 0b0010,
    TYPE_BITS16_TSS_BUSY = 0b0011,
    TYPE_BITS16_CALL_GATE = 0b0100,
    TYPE_TASK_GATE = 0b0101,
    BITS16_INTERRUPT_GATE = 0b0110,
    BITS16_TRAP_GATE = 0b0111,
    TYPE_SYSTEM_RESERVED1 = 0b1000,
    TYPE_BITS32_TSS_AVAILABLE = 0b1001,
    TYPE_SYSTEM_RESERVED2 = 0b1010,
    TYPE_BITS32_TSS_BUSY = 0b1011,
    TYPE_BITS32_CALL_GATE = 0b1100,
    TYPE_SYSTEM_RESERVED3 = 0b1101,
    TYPE_BITS32_INTERRUPT = 0b1110,
    TYPE_BITS32_TRAP_GATE = 0b1111
};

enum descriptor_type_t {
    DESCRIPTOR_SYSTEM = 0,
    DESCRIPTOR_CODE_OR_DATA = 1
};

enum default_op_size_t {
    OP_SIZE_16 = 0,
    OP_SIZE_32 = 1
};

enum granularity_t {
    GRANULARITY_BYTE = 0,
    GRANULARITY_PAGE = 1
};

struct segment_selector_t {
    union {
        struct {
            uint16_t request_privilege_level : 2;
            uint16_t table : 1;
            uint16_t index : 13;
        } bits;

        uint16_t raw;
    };
} __attribute__((packed));

struct segment_descriptor_t {
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

#ifdef X86_64
            uint64_t base_address_upper : 32;
            uint64_t must_be_zero : 32;
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
} __attribute__((packed));

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
} __attribute__((packed));

void write(const segment_table_t &gdt) noexcept;
void store(segment_table_t& gdt) noexcept;

#ifdef _DEBUG
namespace debug {
const wchar_t* to_string(selector_table_t selector_table) noexcept;
const wchar_t* to_string(data_type_t data_type) noexcept;
const wchar_t* to_string(code_type_t code_type) noexcept;
const wchar_t* to_string(system_type_t system_type) noexcept;
const wchar_t* to_string(descriptor_type_t descriptor_type) noexcept;
const wchar_t* to_string(default_op_size_t default_op_size) noexcept;
const wchar_t* to_string(granularity_t granularity) noexcept;
const wchar_t* type_to_string(const segment_descriptor_t& descriptor);
}
#endif

static_assert(sizeof(segment_selector_t) == 2, "segment_selector_t != 2");

#ifdef X86_64
static_assert(sizeof(segment_descriptor_t) == 16, "segment_descriptor_t != 16");
static_assert(sizeof(segment_table_t) == 10, "segment_table_t != 10");
#else
static_assert(sizeof(segment_descriptor_t) == 8, "segment_descriptor_t != 8");
static_assert(sizeof(segment_table_t) == 6, "segment_table_t != 6");
#endif

}
