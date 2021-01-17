#include "x86/intrinsic.h"

#include "x86/msr.h"


uint64_t x86::msr::read(id_t msr_id) noexcept {
    return _read_msr(msr_id);
}

void x86::msr::write(id_t msr_id, uint64_t value) noexcept {
    _write_msr(msr_id, value);
}
