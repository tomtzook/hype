
#include "commonefi.h"

#include <debug.h>

DEBUG_DECL(common,
void trace(const wchar_t* fmt, ...) noexcept {
    va_list args;
    va_start(args, fmt);

    VPrint((CHAR16*)fmt, args);

    va_end(args);
}

void deadloop() noexcept {
    int wait = 1;
    while (wait) {
        __asm__ __volatile__("hlt");
    }
}

const wchar_t* to_string(const result_t& result) noexcept {
    switch (result.category()) {
        case common::result::COMMON: return common_error_to_string(result);
        case hype::result::CATEGORY: return hype::result::debug::to_string(result);
        case efi::result::CATEGORY: return efi::result::debug::to_string(result);
        default: return L"";
    }
}
)
