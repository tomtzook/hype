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
    return !is_system() && bits.type < CODE_EXECUTE_ONLY;
}

bool x86::segment_descriptor_t::is_code() const noexcept {
    return !is_system() && bits.type >= CODE_EXECUTE_ONLY;
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

void x86::read(segment_table_t& gdt) noexcept {
    _sgdt(&gdt);
}
