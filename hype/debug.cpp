
#include "base.h"
#include "efi_base.h"
#include "debug.h"

#ifdef _DEBUG

namespace hype::debug {

void trace(const wchar_t* fmt, ...) {
    wchar_t _print_buffer[255];

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
    //TRACE_DEBUG("Entering Deadloop");

    static volatile int wait = 1;
    while (wait) {
        __asm__ __volatile__("pause");
    }

    //TRACE_DEBUG("Leaving Deadloop");
}

}

#endif
