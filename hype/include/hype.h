#pragma once


#include "common.h"


namespace hype {

struct context_t;

common::result initialize(context_t*& context) noexcept;
common::result free(context_t* context) noexcept;

}
