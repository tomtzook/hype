
#include <cstddef>

#include <x86/segment.h>

#include "debug.h"


void hype::debug::trace(const x86::segment_table_t& table) {
    size_t descriptor_count = table.limit() / sizeof(x86::segment_descriptor_t);
    TRACE_DEBUG("--GDT-- BASE=%x  LIMIT=%d  COUNT=%d --", table.base_address(), table.limit(), descriptor_count);

    for (size_t i = 0; i < descriptor_count; i++) {
        const x86::segment_descriptor_t& descriptor = table[i];

        TRACE_DEBUG("(%d) BASE=%x LIMIT=%d DT=%s T=%s PRIV=%d PRES=%d AVAIL=%d LONG=%d D/B=%s GRAN=%s",
                    i, descriptor.base_address(), descriptor.limit(),
                    x86::debug::to_string(descriptor.descriptor_type()),
                    x86::debug::type_to_string(descriptor),
                    descriptor.bits.privilege,
                    descriptor.bits.present,
                    descriptor.bits.available,
                    descriptor.bits.long_mode,
                    x86::debug::to_string(descriptor.default_big()),
                    x86::debug::to_string(descriptor.granularity()));
    }
}
