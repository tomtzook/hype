#include "x86/intrinsic.h"

#include "x86/segment.h"

void x86::gdtr_t::load() noexcept {
    _lgdt(this);
}

void x86::gdtr_t::store() noexcept {
    _sgdt(this);
}
