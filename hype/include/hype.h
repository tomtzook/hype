#pragma once


#include "common.h"


namespace hype {

struct context_t;

common::result initialize(context_t*& context) noexcept;

common::result start(context_t* context) noexcept;

void free(context_t* context) noexcept;

}
