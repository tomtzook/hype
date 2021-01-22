#pragma once

#include "huge_paging.h"


namespace x86::paging {

enum class mode_t {
    BIT32_PAGING = 0,
    PAE_PAGING,
    IA32E_PAGING
};

mode_t get_mode() noexcept;

}
