#pragma once

#include "base.h"
#include "memory.h"


namespace hype {

framework::result<> initialize();
framework::result<> start();
void free();

}
