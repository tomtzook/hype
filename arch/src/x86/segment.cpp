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

void x86::gdtr_t::load() noexcept {
    _lgdt(this);
}

void x86::gdtr_t::store() noexcept {
    _sgdt(this);
}
