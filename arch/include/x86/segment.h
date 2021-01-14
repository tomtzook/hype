#pragma once

#include "types.h"


namespace x86 {

struct gdtr_t {
    uint16_t limit;
    uint32_t base;

    void load() noexcept;
    void store() noexcept;
};

struct segment_descriptor_t {

};

}
