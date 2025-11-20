#pragma once

#include "type_traits.h"

namespace framework {

template<typename T>
concept trivially_copyable = is_trivially_copyable_v<T>;

template<typename T>
concept trivially_destructible = is_trivially_destructible_v<T>;

}
