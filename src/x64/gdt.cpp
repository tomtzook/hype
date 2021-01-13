#include "x64/intrinsic.h"
#include "x64/gdt.h"

void x64::gdtr_t::load() {
    lgdt(this);
}

void x64::gdtr_t::store() {
    sgdt(this);
}
