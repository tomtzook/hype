
#include "base.h"
#include "efi_base.h"
#include "environment.h"

namespace framework::debug {

static void print_pre_info() {
    const auto cpu_id = environment::get_current_vcpu_id();
    Print(reinterpret_cast<const UINT16*>(L"[%d] "), cpu_id);
}

void trace_impl(const wchar_t* fmt, ...) {
    //print_pre_info();

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

}
