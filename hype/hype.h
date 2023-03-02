#pragma once

#include <common/common.h>
#include <environment/paging.h>


namespace hype {

struct context_t {
    environment::page_table page_table page_aligned;
};

extern context_t* g_context;

result::status initialize() noexcept;
result::status start() noexcept;
void free() noexcept;

}
