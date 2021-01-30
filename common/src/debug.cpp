#include "error.h"

#include "debug.h"


#ifdef _DEBUG
namespace common::debug {
const wchar_t* common_error_to_string(const common::result& result) noexcept {
    switch (result.code()) {
        case common::result::SUCCESS: return L"SUCCESS";
        case common::result::ALLOCATION_ERROR: return L"ALLOCATION_ERROR";
        case common::result::ASSERTION_ERROR: return L"ASSERTION_ERROR";
        case common::result::UNSUPPORTED: return L"UNSUPPORTED";
        default: return L"";
    }
}
}
#endif
