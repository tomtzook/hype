#include "x86/intrinsic.h"

#include "x86/regs.h"

x86::cr0_t::cr0_t() noexcept {
    load();
}

x86::cr0_t::cr0_t(uint64_t raw) noexcept {
    this->raw = raw;
}

void x86::cr0_t::clear() noexcept {
    raw = 0;
}

void x86::cr0_t::load() noexcept {
    raw = _read_cr0();
}

void x86::cr0_t::store() noexcept {
    _write_cr0(raw);
}

x86::cr4_t::cr4_t() noexcept {
    load();
}

x86::cr4_t::cr4_t(uint64_t raw) noexcept {
    this->raw = raw;
}

void x86::cr4_t::clear() noexcept {
    raw = 0;
}

void x86::cr4_t::load() noexcept {
    raw = _read_cr4();
}

void x86::cr4_t::store() noexcept {
    _write_cr4(raw);
}
