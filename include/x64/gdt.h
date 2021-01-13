#pragma once

#include "types.h"


namespace x64 {

struct gdtr_t {
    uint32_t limit : 16;
    uint32_t base : 32;

    void load();
    void store();
};

struct segment_descriptor_t {

};

}
