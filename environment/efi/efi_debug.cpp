
#include <lock.h>

#include "base.h"
#include "efi_base.h"
#include "environment.h"

namespace environment {
extern bool g_trace_enabled_for_core[32];
}

namespace framework::debug {

static framework::spin_lock& trace_lock() {
    static framework::spin_lock lock{};
    return lock;
}

static size_t next_trace_count() {
    static framework::atomic<size_t> count{0};
    return count.fetch_add(1);
}

void trace_impl(const wchar_t* fmt, ...) {
    const auto core_id = environment::get_current_vcpu_id();
    if (!environment::g_trace_enabled_for_core[core_id]) {
        return;
    }

    // todo: fix
    //framework::unique_lock<spin_lock> unique_lock(trace_lock());
    //const auto trace_count = next_trace_count();

    wchar_t _print_buffer[512];

    const auto count = UnicodeSPrint(
        reinterpret_cast<UINT16*>(_print_buffer),
        sizeof(_print_buffer),
        reinterpret_cast<const UINT16*>(L"+%d [%d] "), 0, core_id);

    const auto count_wide = count * sizeof(wchar_t);
    if (count_wide < sizeof(_print_buffer)) {
        VA_LIST args;
        VA_START(args, fmt);

        UnicodeVSPrint(reinterpret_cast<UINT16*>(_print_buffer + count),
                       sizeof(_print_buffer) - count_wide,
                       reinterpret_cast<const UINT16*>(fmt),
                       args);

        VA_END(args);
    }

#ifdef trace_screen
    Print(reinterpret_cast<UINT16*>(_print_buffer));
#else
    char _ascii_print_buffer[sizeof(_print_buffer) / sizeof(wchar_t)];
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
