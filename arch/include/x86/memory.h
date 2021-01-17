#pragma once

#include <types.h>

#define PAGE_ALIGNED __attribute__ ((aligned (x86::PAGE_SIZE)));

namespace x86 {

const size_t PAGE_SIZE = 0x1000;

size_t get_maximum_physical_address_width() noexcept;

bool is_page_aligned(void* ptr) noexcept;
constexpr bool is_page_aligned(uintn_t address) noexcept;

size_t get_address_size(uintn_t address) noexcept;
bool does_address_in_max_physical_width(uintn_t address) noexcept;

}
