

#include "x86/error.h"
#include "x86/segment.h"
#include "x86/cr.h"


#ifdef _DEBUG
namespace x86::debug {
const wchar_t* to_string(selector_table_t selector_table) noexcept {
    switch (selector_table) {
        case TABLE_GDT: return L"TABLE_GDT";
        case TABLE_LDT: return L"TABLE_LDT";
        default: return L"";
    }
}

const wchar_t* to_string(segment_type_data_code_t segment_type) noexcept {
    switch (segment_type) {
        case DATA_READ_ONLY: return L"DATA_READ_ONLY";
        case DATA_READ_ONLY_ACCESSED: return L"DATA_READ_ONLY_ACCESSED";
        case DATA_READ_WRITE: return L"DATA_READ_WRITE";
        case DATA_READ_WRITE_ACCESSED: return L"DATA_READ_WRITE_ACCESSED";
        case DATA_READ_ONLY_EXPAND_DOWN: return L"DATA_READ_ONLY_EXPAND_DOWN";
        case DATA_READ_ONLY_EXPAND_DOWN_ACCESSED: return L"DATA_READ_ONLY_EXPAND_DOWN_ACCESSED";
        case DATA_READ_WRITE_EXPAND_DOWN: return L"DATA_READ_WRITE_EXPAND_DOWN";
        case DATA_READ_WRITE_EXPAND_DOWN_ACCESSED: return L"DATA_READ_WRITE_EXPAND_DOWN_ACCESSED";
        case CODE_EXECUTE_ONLY: return L"CODE_EXECUTE_ONLY";
        case CODE_EXECUTE_ONLY_ACCESSED: return L"CODE_EXECUTE_ONLY_ACCESSED";
        case CODE_EXECUTE_READ: return L"CODE_EXECUTE_READ";
        case CODE_EXECUTE_READ_ACCESSED: return L"CODE_EXECUTE_READ_ACCESSED";
        case CODE_EXECUTE_ONLY_CONFORMING: return L"CODE_EXECUTE_ONLY_CONFORMING";
        case CODE_EXECUTE_ONLY_CONFORMING_ACCESSED: return L"CODE_EXECUTE_ONLY_CONFORMING_ACCESSED";
        case CODE_EXECUTE_READ_CONFORMING: return L"CODE_EXECUTE_READ_CONFORMING";
        case CODE_EXECUTE_READ_CONFORMING_ACCESSED: return L"CODE_EXECUTE_READ_CONFORMING_ACCESSED";
        default: return L"";
    }
}

const wchar_t* to_string(segment_type_system_t segment_type) noexcept {
    switch (segment_type) {
        case SYSTEM_RESERVED0: return L"SYSTEM_RESERVED0";
        case SYSTEM_BITS16_TSS_AVAILABLE: return L"TYPE_BITS16_TSS_AVAILABLE";
        case SYSTEM_LDT: return L"TYPE_LDT";
        case SYSTEM_BITS16_TSS_BUSY: return L"TYPE_BITS16_TSS_BUSY";
        case SYSTEM_BITS16_CALL_GATE: return L"TYPE_BITS16_CALL_GATE";
        case SYSTEM_TASK_GATE: return L"TYPE_TASK_GATE";
        case SYSTEM_BITS16_INTERRUPT_GATE: return L"BITS16_INTERRUPT_GATE";
        case SYSTEM_BITS16_TRAP_GATE: return L"BITS16_TRAP_GATE";
        case SYSTEM_RESERVED1: return L"TYPE_SYSTEM_RESERVED1";
        case SYSTEM_BITS32_TSS_AVAILABLE: return L"TYPE_BITS32_TSS_AVAILABLE";
        case SYSTEM_RESERVED2: return L"TYPE_SYSTEM_RESERVED2";
        case SYSTEM_BITS32_TSS_BUSY: return L"TYPE_BITS32_TSS_BUSY";
        case SYSTEM_BITS32_CALL_GATE: return L"TYPE_BITS32_CALL_GATE";
        case SYSTEM_RESERVED3: return L"TYPE_SYSTEM_RESERVED3";
        case SYSTEM_BITS32_INTERRUPT: return L"TYPE_BITS32_INTERRUPT";
        case SYSTEM_BITS32_TRAP_GATE: return L"TYPE_BITS32_TRAP_GATE";
        default: return L"";
    }
}

const wchar_t* to_string(descriptor_type_t descriptor_type) noexcept {
    switch (descriptor_type) {
        case DESCRIPTOR_SYSTEM: return L"DESCRIPTOR_SYSTEM";
        case DESCRIPTOR_CODE_OR_DATA: return L"DESCRIPTOR_CODE_OR_DATA";
        default: return L"";
    }
}

const wchar_t* to_string(default_op_size_t default_op_size) noexcept {
    switch (default_op_size) {
        case OP_SIZE_16: return L"OP_SIZE_16";
        case OP_SIZE_32: return L"OP_SIZE_32";
        default: return L"";
    }
}

const wchar_t* to_string(granularity_t granularity) noexcept {
    switch (granularity) {
        case GRANULARITY_BYTE: return L"GRANULARITY_BYTE";
        case GRANULARITY_PAGE: return L"GRANULARITY_PAGE";
        default: return L"";
    }
}

const wchar_t* type_to_string(const segment_descriptor_t& descriptor) noexcept {
    if (descriptor.is_system()) {
        return to_string(static_cast<segment_type_system_t>(descriptor.bits.type));
    } else {
        return to_string(static_cast<segment_type_data_code_t>(descriptor.bits.type));
    }
}

void trace(const segment_descriptor_t& descriptor) noexcept {
    TRACE_DEBUG("BASE=%x LIMIT=%d DT=%s T=%s PRIV=%d PRES=%d AVAIL=%d LONG=%d D/B=%s GRAN=%s",
                descriptor.base_address(), descriptor.limit(),
                to_string(descriptor.descriptor_type()),
                type_to_string(descriptor),
                descriptor.bits.privilege,
                descriptor.bits.present,
                descriptor.bits.available,
                descriptor.bits.long_mode,
                to_string(descriptor.default_big()),
                to_string(descriptor.granularity()));
}

void trace(const segment_table_t& table) noexcept {
    size_t descriptor_count = table.limit() / sizeof(x86::segment_descriptor_t);
    TRACE_DEBUG("--GDT-- BASE=%x  LIMIT=%d  COUNT=%d --", table.base_address(), table.limit(), descriptor_count);

    for (size_t i = 0; i < descriptor_count; i++) {
        const x86::segment_descriptor_t& descriptor = table[i];

        TRACE_DEBUG("(%d) BASE=%x LIMIT=%d DT=%s T=%s PRIV=%d PRES=%d AVAIL=%d LONG=%d D/B=%s GRAN=%s",
                    i, descriptor.base_address(), descriptor.limit(),
                    to_string(descriptor.descriptor_type()),
                    type_to_string(descriptor),
                    descriptor.bits.privilege,
                    descriptor.bits.present,
                    descriptor.bits.available,
                    descriptor.bits.long_mode,
                    to_string(descriptor.default_big()),
                    to_string(descriptor.granularity()));

    }

    TRACE_DEBUG("--END--GDT--------------------------");
}
} // x86::debug

namespace x86::result::debug {
const wchar_t* to_string(const common::result& result) noexcept {
    switch (result.code()) {
        case x86::result::SUCCESS: return L"SUCCESS";
        case x86::result::STATE_NOT_SUPPORTED: return L"STATE_NOT_SUPPORTED";
        case x86::result::INSTRUCTION_FAILED: return L"INSTRUCTION_FAILED";
        default: return L"";
    }
}
} // x86::result::debug
#endif
