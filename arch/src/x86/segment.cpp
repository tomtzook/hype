#include <types.h>
#include <debug.h>

#include "x86/intrinsic.h"
#include "x86/segment.h"


x86::segment_descriptor_t::segment_descriptor_t() {
#ifdef X86_64
    bits.must_be_zero = 0;
#endif
}

void* x86::segment_descriptor_t::base_address() const noexcept {
    uintn_t address = static_cast<uintn_t>(bits.base_address_low) |
            (static_cast<uintn_t>(bits.base_address_middle) << 16) |
            (static_cast<uintn_t>(bits.base_address_high) << 24);

#ifdef X86_64
    if (descriptor_type_t::DESCRIPTOR_SYSTEM == bits.descriptor_type) {
        address |= (static_cast<uintn_t>(bits.base_address_upper) << 32);
    }
#endif

    return reinterpret_cast<void*>(address);
}

void x86::segment_descriptor_t::base_address(void* value) noexcept {
    uintn_t address = reinterpret_cast<uintn_t>(value);
    bits.base_address_low = address & 0xffff;
    bits.base_address_middle = (address >> 16) & 0xff;
    bits.base_address_high = (address >> 24) & 0xff;

#ifdef X86_64
    bits.base_address_upper = (address >> 32) & 0xffff;
#endif
}

uint32_t x86::segment_descriptor_t::limit() const noexcept {
    return static_cast<uint32_t>(bits.limit_low) |
            (static_cast<uint32_t>(bits.limit_high) << 16);
}

void x86::segment_descriptor_t::limit(uint32_t value) noexcept {
    bits.limit_low = value & 0xffff;
    bits.limit_high = (value >> 16) & 0xf;
}

bool x86::segment_descriptor_t::is_system() const noexcept {
    return DESCRIPTOR_SYSTEM == bits.descriptor_type;
}

bool x86::segment_descriptor_t::is_data() const noexcept {
    return !is_system() && bits.type < EXECUTE_ONLY;
}

bool x86::segment_descriptor_t::is_code() const noexcept {
    return !is_system() && bits.type >= EXECUTE_ONLY;
}

x86::descriptor_type_t x86::segment_descriptor_t::descriptor_type() const noexcept {
    return static_cast<descriptor_type_t>(bits.descriptor_type);
}

void x86::segment_descriptor_t::descriptor_type(descriptor_type_t descriptor_type) noexcept {
    bits.descriptor_type = descriptor_type;
}

x86::default_op_size_t x86::segment_descriptor_t::default_big() const noexcept {
    return static_cast<default_op_size_t>(bits.default_big);
}

void x86::segment_descriptor_t::default_big(default_op_size_t default_big) noexcept {
    bits.default_big = default_big;
}

x86::granularity_t x86::segment_descriptor_t::granularity() const noexcept {
    return static_cast<granularity_t>(bits.granularity);
}

void x86::segment_descriptor_t::granularity(granularity_t granularity) noexcept {
    bits.granularity = granularity;
}

const x86::segment_descriptor_t& x86::segment_table_t::operator[](const segment_selector_t& selector) const noexcept {
    return *reinterpret_cast<segment_descriptor_t*>(reinterpret_cast<uintn_t>(base_address()) + selector.bits.index * 8);
}

x86::segment_descriptor_t& x86::segment_table_t::operator[](const segment_selector_t& selector) noexcept {
    return *reinterpret_cast<segment_descriptor_t*>(reinterpret_cast<uintn_t>(base_address()) + selector.bits.index * 8);
}

const x86::segment_descriptor_t& x86::segment_table_t::operator[](int index) const noexcept {
    return reinterpret_cast<segment_descriptor_t*>(base_address())[index];
}

x86::segment_descriptor_t& x86::segment_table_t::operator[](int index) noexcept {
    return reinterpret_cast<segment_descriptor_t*>(base_address())[index];
}

void* x86::segment_table_t::base_address() const noexcept {
    return reinterpret_cast<void*>(base);
}

uint16_t x86::segment_table_t::limit() const noexcept {
    return lim;
}

void x86::write(const segment_table_t &gdt) noexcept {
    _lgdt(const_cast<segment_table_t*>(&gdt));
}

void x86::store(segment_table_t& gdt) noexcept {
    _sgdt(&gdt);
}

#ifdef _DEBUG
const wchar_t* x86::debug::to_string(selector_table_t selector_table) noexcept {
    switch (selector_table) {
        case TABLE_GDT: return L"TABLE_GDT";
        case TABLE_LDT: return L"TABLE_LDT";
        default: return L"";
    }
}

const wchar_t* x86::debug::to_string(data_type_t data_type) noexcept {
    switch (data_type) {
        case READ_ONLY: return L"READ_ONLY";
        case READ_ONLY_ACCESSED: return L"READ_ONLY_ACCESSED";
        case READ_WRITE: return L"READ_WRITE";
        case READ_WRITE_ACCESSED: return L"READ_WRITE_ACCESSED";
        case READ_ONLY_EXPAND_DOWN: return L"READ_ONLY_EXPAND_DOWN";
        case READ_ONLY_EXPAND_DOWN_ACCESSED: return L"READ_ONLY_EXPAND_DOWN_ACCESSED";
        case READ_WRITE_EXPAND_DOWN: return L"READ_WRITE_EXPAND_DOWN";
        case READ_WRITE_EXPAND_DOWN_ACCESSED: return L"READ_WRITE_EXPAND_DOWN_ACCESSED";
        default: return L"";
    }
}

const wchar_t* x86::debug::to_string(code_type_t code_type) noexcept {
    switch (code_type) {
        case EXECUTE_ONLY: return L"EXECUTE_ONLY";
        case EXECUTE_ONLY_ACCESSED: return L"EXECUTE_ONLY_ACCESSED";
        case EXECUTE_READ: return L"EXECUTE_READ";
        case EXECUTE_READ_ACCESSED: return L"EXECUTE_READ_ACCESSED";
        case EXECUTE_ONLY_CONFORMING: return L"EXECUTE_ONLY_CONFORMING";
        case EXECUTE_ONLY_CONFORMING_ACCESSED: return L"EXECUTE_ONLY_CONFORMING_ACCESSED";
        case EXECUTE_READ_CONFORMING: return L"EXECUTE_READ_CONFORMING";
        case EXECUTE_READ_CONFORMING_ACCESSED: return L"EXECUTE_READ_CONFORMING_ACCESSED";
        default: return L"";
    }
}

const wchar_t* x86::debug::to_string(system_type_t system_type) noexcept {
    switch (system_type) {
        case TYPE_SYSTEM_RESERVED0: return L"TYPE_SYSTEM_RESERVED0";
        case TYPE_BITS16_TSS_AVAILABLE: return L"TYPE_BITS16_TSS_AVAILABLE";
        case TYPE_LDT: return L"TYPE_LDT";
        case TYPE_BITS16_TSS_BUSY: return L"TYPE_BITS16_TSS_BUSY";
        case TYPE_BITS16_CALL_GATE: return L"TYPE_BITS16_CALL_GATE";
        case TYPE_TASK_GATE: return L"TYPE_TASK_GATE";
        case BITS16_INTERRUPT_GATE: return L"BITS16_INTERRUPT_GATE";
        case BITS16_TRAP_GATE: return L"BITS16_TRAP_GATE";
        case TYPE_SYSTEM_RESERVED1: return L"TYPE_SYSTEM_RESERVED1";
        case TYPE_BITS32_TSS_AVAILABLE: return L"TYPE_BITS32_TSS_AVAILABLE";
        case TYPE_SYSTEM_RESERVED2: return L"TYPE_SYSTEM_RESERVED2";
        case TYPE_BITS32_TSS_BUSY: return L"TYPE_BITS32_TSS_BUSY";
        case TYPE_BITS32_CALL_GATE: return L"TYPE_BITS32_CALL_GATE";
        case TYPE_SYSTEM_RESERVED3: return L"TYPE_SYSTEM_RESERVED3";
        case TYPE_BITS32_INTERRUPT: return L"TYPE_BITS32_INTERRUPT";
        case TYPE_BITS32_TRAP_GATE: return L"TYPE_BITS32_TRAP_GATE";
        default: return L"";
    }
}

const wchar_t* x86::debug::to_string(descriptor_type_t descriptor_type) noexcept {
    switch (descriptor_type) {
        case DESCRIPTOR_SYSTEM: return L"DESCRIPTOR_SYSTEM";
        case DESCRIPTOR_CODE_OR_DATA: return L"DESCRIPTOR_CODE_OR_DATA";
        default: return L"";
    }
}

const wchar_t* x86::debug::to_string(default_op_size_t default_op_size) noexcept {
    switch (default_op_size) {
        case OP_SIZE_16: return L"OP_SIZE_16";
        case OP_SIZE_32: return L"OP_SIZE_32";
        default: return L"";
    }
}

const wchar_t* x86::debug::to_string(granularity_t granularity) noexcept {
    switch (granularity) {
        case GRANULARITY_BYTE: return L"GRANULARITY_BYTE";
        case GRANULARITY_PAGE: return L"GRANULARITY_PAGE";
        default: return L"";
    }
}

const wchar_t* x86::debug::type_to_string(const segment_descriptor_t& descriptor) noexcept {
    if (descriptor.is_system()) {
        return to_string(static_cast<system_type_t>(descriptor.bits.type));
    } else if (descriptor.is_data()) {
        return to_string(static_cast<data_type_t>(descriptor.bits.type));
    } else {
        return to_string(static_cast<code_type_t>(descriptor.bits.type));
    }
}

void x86::debug::trace(const segment_descriptor_t& descriptor) noexcept {
    TRACE_DEBUG("BASE=%x LIMIT=%d DT=%s T=%s PRIV=%d PRES=%d AVAIL=%d LONG=%d D/B=%s GRAN=%s",
                descriptor.base_address(), descriptor.limit(),
                x86::debug::to_string(descriptor.descriptor_type()),
                x86::debug::type_to_string(descriptor),
                descriptor.bits.privilege,
                descriptor.bits.present,
                descriptor.bits.available,
                descriptor.bits.long_mode,
                x86::debug::to_string(descriptor.default_big()),
                x86::debug::to_string(descriptor.granularity()));
}

void x86::debug::trace(const segment_table_t& table) noexcept {
    size_t descriptor_count = table.limit() / sizeof(x86::segment_descriptor_t);
    TRACE_DEBUG("--GDT-- BASE=%x  LIMIT=%d  COUNT=%d --", table.base_address(), table.limit(), descriptor_count);

    for (size_t i = 0; i < descriptor_count; i++) {
        const x86::segment_descriptor_t& descriptor = table[i];

        TRACE_DEBUG("(%d) BASE=%x LIMIT=%d DT=%s T=%s PRIV=%d PRES=%d AVAIL=%d LONG=%d D/B=%s GRAN=%s",
                    i, descriptor.base_address(), descriptor.limit(),
                    x86::debug::to_string(descriptor.descriptor_type()),
                    x86::debug::type_to_string(descriptor),
                    descriptor.bits.privilege,
                    descriptor.bits.present,
                    descriptor.bits.available,
                    descriptor.bits.long_mode,
                    x86::debug::to_string(descriptor.default_big()),
                    x86::debug::to_string(descriptor.granularity()));

    }

    TRACE_DEBUG("--END--GDT--------------------------");
}
#endif
