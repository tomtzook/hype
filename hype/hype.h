#pragma once

#include "base.h"
#include "memory.h"


namespace hype {

status_t initialize() noexcept;
status_t start() noexcept;
void free() noexcept;

}
