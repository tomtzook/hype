#pragma once

#include "types.h"


namespace x86 {

struct gdtr_t {
    uint32_t limit : 16;
    uint32_t base : 32;

    void load() noexcept;
    void store() noexcept;
};

struct segment_descriptor_t {

};

}
