
#include "commonefi.h"

#include <debug.h>

#ifdef _DEBUG

void common::debug::trace(const wchar_t* fmt, ...) noexcept {
    va_list args;
    va_start(args, fmt);

    VPrint((CHAR16*)fmt, args);

    va_end(args);
}

void common::debug::deadloop() noexcept {
    int wait = 1;
    while (wait) {
        __asm__ __volatile__("hlt");
    }
}

const wchar_t* common::debug::to_string(const result_t& result) noexcept {
    switch (result.category()) {
        case common::result::COMMON: return common::debug::common_error_to_string(result);
        case hype::result::CATEGORY: return hype::result::debug::to_string(result);
        case efi::result::CATEGORY: return efi::result::debug::to_string(result);
        default: return L"";
    }
}

#endif
