#pragma once


#include "error.h"


namespace hype {

struct context_t;

hype::result initialize(context_t*& context) noexcept;
hype::result free(context_t* context) noexcept;

}
