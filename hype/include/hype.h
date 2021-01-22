#pragma once

#include <x86/paging.h>

#include "common.h"
#include "vcpu.h"
#include "env.h"


namespace hype {

struct context_t {
    environment_t environment;

    x86::paging::huge_page_table_t page_table PAGE_ALIGNED;
};

extern context_t* g_context;


common::result initialize() noexcept;
common::result start() noexcept;
void free() noexcept;

}
