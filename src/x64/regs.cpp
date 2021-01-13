#include "x64/intrinsic.h"

#include "x64/regs.h"

x64::cr0_t::cr0_t() {
    load();
}

x64::cr0_t::cr0_t(uint64_t raw) {
    this->raw = raw;
}

void x64::cr0_t::clear() {
    raw = 0;
}

void x64::cr0_t::load() {
    raw = read_cr0();
}

void x64::cr0_t::store() {
    write_cr0(raw);
}

x64::cr4_t::cr4_t() {
    load();
}

x64::cr4_t::cr4_t(uint64_t raw) {
    this->raw = raw;
}

void x64::cr4_t::clear() {
    raw = 0;
}

void x64::cr4_t::load() {
    raw = read_cr4();
}

void x64::cr4_t::store() {
    write_cr4(raw);
}
