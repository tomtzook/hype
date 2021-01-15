#include "x86/intrinsic.h"

#include "x86/segment.h"

void* x86::segment_descriptor_t::base_address() const noexcept {
    uint64_t address = static_cast<uint64_t>(bits.base_address_low) |
            (static_cast<uint64_t>(bits.base_address_middle) << 16) |
            (static_cast<uint64_t>(bits.base_address_high) << 24);

    return reinterpret_cast<void*>(address);
}

void x86::segment_descriptor_t::base_address(void* value) noexcept {
    uint64_t address = reinterpret_cast<uint64_t>(value);
    bits.base_address_low = address & 0xffff;
    bits.base_address_middle = (address >> 16) & 0xff;
    bits.base_address_high = (address >> 24) & 0xff;
}

uint32_t x86::segment_descriptor_t::limit() const noexcept {
    return static_cast<uint32_t>(bits.limit_low) |
            (static_cast<uint32_t>(bits.limit_high) << 16);
}

void x86::segment_descriptor_t::limit(uint32_t value) noexcept {
    bits.limit_low = value & 0xffff;
    bits.limit_high = (value >> 16) & 0xf;
}

x86::segment_descriptor64_t::segment_descriptor64_t() {
    upper.bits.must_be_zero = 0;
}

void* x86::segment_descriptor64_t::base_address() const noexcept {
    void* address_base = segment_descriptor_t::base_address();
    uint64_t address = reinterpret_cast<uint64_t>(address_base);

    if (descriptor_type_t::DESCRIPTOR_SYSTEM == bits.descriptor_type) {
        address |= (static_cast<uint64_t>(upper.bits.base_address_upper) << 32);
    }

    return reinterpret_cast<void*>(address);
}

void x86::segment_descriptor64_t::base_address(void* value) noexcept {
    segment_descriptor_t::base_address(value);

    uint64_t address = reinterpret_cast<uint64_t>(value);
    upper.bits.base_address_upper = (address >> 32) & 0xffff;
}

void x86::_segment_table_base_t::load() const noexcept {
    _lgdt(const_cast<_segment_table_base_t*>(this));
}

void x86::_segment_table_base_t::store() noexcept {
    _sgdt(this);
}

const x86::segment_descriptor_t& x86::_segment_table_base_t::operator[](const segment_selector_t& selector) const noexcept {
    return *reinterpret_cast<segment_descriptor_t*>(reinterpret_cast<uint64_t>(base_address()) + selector.bits.index * 8);
}

x86::segment_descriptor_t& x86::_segment_table_base_t::operator[](const segment_selector_t& selector) noexcept {
    return *reinterpret_cast<segment_descriptor_t*>(reinterpret_cast<uint64_t>(base_address()) + selector.bits.index * 8);
}

const x86::segment_descriptor_t& x86::_segment_table_base_t::operator[](int index) const noexcept {
    return reinterpret_cast<segment_descriptor_t*>(base_address())[index];
}

x86::segment_descriptor_t& x86::_segment_table_base_t::operator[](int index) noexcept {
    return reinterpret_cast<segment_descriptor_t*>(base_address())[index];
}

void* x86::segment_table_t::base_address() const noexcept {
    return reinterpret_cast<void*>(base);
}

uint16_t x86::segment_table_t::limit() const noexcept {
    return lim;
}

void* x86::segment_table64_t::base_address() const noexcept {
    return reinterpret_cast<void*>(base);
}

uint16_t x86::segment_table64_t::limit() const noexcept {
    return lim;
}
