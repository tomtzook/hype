
#include <debug.h>
#include "efi_base.h"

namespace framework::debug {

void trace_impl(const wchar_t* fmt, ...) {
    wchar_t _print_buffer[512];

    VA_LIST args;
    VA_START(args, fmt);

    UnicodeVSPrint(reinterpret_cast<UINT16*>(_print_buffer),
                   sizeof(_print_buffer),
                   reinterpret_cast<const UINT16*>(fmt),
                   args);

    VA_END(args);

    Print(reinterpret_cast<UINT16*>(_print_buffer));
}

void deadloop() {
    //trace_debug("Entering Deadloop");

    static volatile int wait = 1;
    while (wait) {
        __asm__ __volatile__("pause");
    }

    //trace_debug("Leaving Deadloop");
}

}
