#include "x86/intrinsic.h"

#include "x86/regs.h"

x86::cr0_t::cr0_t() noexcept {
    store(*this);
}

x86::cr0_t::cr0_t(uint64_t raw) noexcept {
    this->raw = raw;
}

x86::cr3_t::cr3_t() noexcept {
    store(*this);
}

x86::cr3_t::cr3_t(uint64_t raw) noexcept {
    this->raw = raw;
}

x86::cr4_t::cr4_t() noexcept {
    store(*this);
}

x86::cr4_t::cr4_t(uint64_t raw) noexcept {
    this->raw = raw;
}

void x86::load(const cr0_t& reg) noexcept {
    _write_cr0(reg.raw);
}

void x86::store(cr0_t& reg) noexcept {
    reg.raw = _read_cr0();
}

void x86::load(const cr3_t& reg) noexcept {
    _write_cr3(reg.raw);
}

void x86::store(cr3_t& reg) noexcept {
    reg.raw = _read_cr3();
}

void x86::load(const cr4_t& reg) noexcept {
    _write_cr4(reg.raw);
}

void x86::store(cr4_t& reg) noexcept {
    reg.raw = _read_cr4();
}
