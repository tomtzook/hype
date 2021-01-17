
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

#endif
