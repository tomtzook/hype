

#include "memory.h"


namespace hype::environment {

physical_address_t to_physical(void* address) noexcept {
    // one-to-one mapping in EFI
    return reinterpret_cast<physical_address_t>(address);
}

}

