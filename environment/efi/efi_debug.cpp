
#include <lock.h>

#include "base.h"
#include "efi_base.h"

namespace framework::debug {

framework::spin_lock lock;

void trace_impl(const wchar_t* fmt, ...) {
    framework::unique_lock<spin_lock> unique_lock(lock);

    wchar_t _print_buffer[512];

    VA_LIST args;
    VA_START(args, fmt);

    UnicodeVSPrint(reinterpret_cast<UINT16*>(_print_buffer),
                   sizeof(_print_buffer),
                   reinterpret_cast<const UINT16*>(fmt),
                   args);

    VA_END(args);

#ifdef trace_screen
    Print(reinterpret_cast<UINT16*>(_print_buffer));
#else
    char _ascii_print_buffer[sizeof(_print_buffer)];
    UnicodeStrToAsciiStrS(
        reinterpret_cast<UINT16*>(_print_buffer),
        _ascii_print_buffer,
        sizeof(_ascii_print_buffer));
    SerialPortWrite(
        reinterpret_cast<UINT8*>(_ascii_print_buffer),
        AsciiStrLen(_ascii_print_buffer));
#endif
}

}
