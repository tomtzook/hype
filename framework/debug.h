#pragma once


#define trace(fmt, ...) framework::debug::trace_impl(fmt L"\n", ##__VA_ARGS__)

#define trace_debug(fmt, ...) trace(L"[DEBUG] " fmt, ##__VA_ARGS__)
#define trace_error(fmt, ...) trace(L"[ERROR] %s.%d: " fmt L"\n", L"" __FILE__, __LINE__, ##__VA_ARGS__)

namespace framework::debug {

void trace_impl(const wchar_t* fmt, ...);
void deadloop();

}
