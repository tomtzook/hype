#pragma once

#include <types.h>

#define PAGE_ALIGNED __attribute__ ((aligned (x86::PAGE_SIZE)));

namespace x86 {

const uint32_t PAGE_SIZE = 4096;

}
