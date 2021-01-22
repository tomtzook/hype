
#include "x86/intrinsic.h"

#include "x86/cr.h"


x86::cr0_t::cr0_t() noexcept {
    read(*this);
}

x86::cr0_t::cr0_t(uint64_t raw) noexcept {
    this->raw = raw;
}

x86::cr3_t::cr3_t() noexcept {
    read(*this);
}

x86::cr3_t::cr3_t(uint64_t raw) noexcept {
    this->raw = raw;
}

x86::cr4_t::cr4_t() noexcept {
    read(*this);
}

x86::cr4_t::cr4_t(uint64_t raw) noexcept {
    this->raw = raw;
}

void x86::read(cr0_t& reg) noexcept {
    reg.raw = _read_cr0();
}

void x86::write(const cr0_t& reg) noexcept {
    _write_cr0(reg.raw);
}

void x86::read(cr3_t& reg) noexcept {
    reg.raw = _read_cr3();
}

void x86::write(const cr3_t &reg) noexcept {
    _write_cr3(reg.raw);
}

void x86::read(cr4_t& reg) noexcept {
    reg.raw = _read_cr4();
}

void x86::write(const cr4_t &reg) noexcept {
    _write_cr4(reg.raw);
}
