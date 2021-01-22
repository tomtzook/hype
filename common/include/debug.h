#pragma once


#ifdef _DEBUG
#define DEBUG_ONLY(...) do { __VA_ARGS__; } while(0)
#define DEBUG_DECL(top_namespace, ...) namespace top_namespace::debug { __VA_ARGS__; }
#define DEBUG_NAMESPACE_START(top_namespace) namespace top_namespace::debug {
#define DEBUG_NAMESPACE_END }
#else
#define DEBUG_ONLY(...)
#define DEBUG_DECL(top_namespace, ...)
#define DEBUG_NAMESPACE_START(top_namespace)
#define DEBUG_NAMESPACE_END
#endif

#define TRACE(fmt, ...) DEBUG_ONLY(common::debug::trace(fmt "\n", ##__VA_ARGS__);)

#define TRACE_DEBUG(fmt, ...) TRACE(L"[DEBUG] " fmt, ##__VA_ARGS__)
#define TRACE_ERROR(fmt, ...) TRACE(L"[ERROR] %s.%d: " fmt "\n", L"" __FILE__, __LINE__, ##__VA_ARGS__)

#define TRACE_CPUID_REG(leaf, subleaf, reg, value) TRACE_DEBUG("CPUID[%d:%d].%s = 0x%x", leaf, subleaf, L"" reg, value)
#define TRACE_CPUID_BIT(leaf, subleaf, reg, bit, value) TRACE_DEBUG("CPUID[%d:%d].%s.%s = 0x%x", leaf, subleaf, L"" reg, L"" bit, value)
#define TRACE_REG_BIT(reg, bit, value) TRACE_DEBUG("%s.%s = 0x%x", L"" reg, L"" bit, value);


DEBUG_NAMESPACE_START(common);
void trace(const wchar_t* fmt, ...) noexcept;
void deadloop() noexcept;
DEBUG_NAMESPACE_END
