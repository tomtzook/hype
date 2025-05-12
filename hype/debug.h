#pragma once


#ifdef _DEBUG
#define DEBUG_ONLY(...) do { __VA_ARGS__; } while(0)
#else
#define DEBUG_ONLY(...)
#endif

#define TRACE(fmt, ...) DEBUG_ONLY(hype::debug::trace(fmt L"\n", ##__VA_ARGS__);)

#define TRACE_DEBUG(fmt, ...) TRACE(L"[DEBUG] " fmt, ##__VA_ARGS__)
#define TRACE_ERROR(fmt, ...) TRACE(L"[ERROR] %s.%d: " fmt L"\n", L"" __FILE__, __LINE__, ##__VA_ARGS__)


#ifdef _DEBUG

namespace hype::debug {

void trace(const wchar_t* fmt, ...);
void deadloop();

}

#endif
