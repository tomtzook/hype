#pragma once

#include <types.h>

#define PAGE_ALIGNED __attribute__ ((aligned (x86::PAGE_SIZE)));

#define SIZE_1GB (1073741824ull)

#define PAGE_SHIFT_LEFT(value) ((value) << x86::PAGE_BITS)
#define PAGE_SHIFT_RIGHT(value) ((value) >> x86::PAGE_BITS)

namespace x86 {

constexpr size_t PAGE_SIZE = 0x1000;
constexpr size_t PAGE_BITS = 12;

size_t get_maximum_physical_address_width() noexcept;

bool is_page_aligned(void* ptr) noexcept;

constexpr bool is_page_aligned(physical_address_t address) noexcept {
    return 0 == (address & (PAGE_SIZE - 1));
}

size_t get_address_size(physical_address_t address) noexcept;
bool is_address_in_max_physical_width(physical_address_t address) noexcept;

}
