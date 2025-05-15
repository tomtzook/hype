#pragma once

#include "x86/types.h"
#include "debug.h"

namespace hype {
namespace status {

enum class category_t {
    hype,
    efi,
    vmx_instruction
};

using code_t = uint64_t;

enum hype_code_t {
    success = 0,
    error_allocation,
    error_unsupported,
    error_assert,
    error_bad_argument,
};

class status_t {
public:
    status_t();
    status_t(category_t category, code_t code);
    status_t(hype_code_t code);
    status_t(status_t&& other);

    [[nodiscard]] category_t category() const;
    [[nodiscard]] code_t code() const;

    [[nodiscard]] bool success() const;

    explicit operator bool() const;

    status_t& operator=(const status_t& other);
    status_t& operator=(status_t&& other);

private:
    category_t m_category;
    code_t m_code;
};

#define TRACE_STATUS(__status) \
    do {   \
        if (!__status) { \
            TRACE_ERROR("%s:%d: ErrorStatus{category=0x%x code=0x%x}", __FILE__, __LINE__, __status.category(), __status.code());\
        } \
    } while(0)

#define CHECK(...) \
    do {            \
        auto __status = hype::status::status_t {__VA_ARGS__}; \
        if (!__status) { \
            TRACE_STATUS(__status); \
            return __status; \
        } \
    } while(0)


#define CHECK_ASSERT(assertion, message) \
    do {                           \
        if (!(assertion)) {          \
            TRACE_ERROR("Assertion Error: %s", L"" message); \
            CHECK(hype::status::error_assert); \
        } \
    } while(0)

#define CHECK_ALLOCATION(__variable, message) \
    do {                           \
        if (nullptr == __variable) {          \
            TRACE_ERROR("Allocation failed: %s", L"" message); \
            CHECK(hype::status::error_allocation); \
        } \
    } while(0)

#define CHECK_AND_JUMP(__label, __status_var, ...) \
    do {                             \
        auto __status = hype::status::status_t {__VA_ARGS__}; \
        if (!__status) {          \
            TRACE_STATUS(__status);  \
            __status_var = __status; \
            goto __label; \
        } \
    } while(0)

#define CHECK_VMX(...) \
    do {                             \
        auto __status = hype::status::status_t { \
            hype::status::category_t::vmx_instruction, static_cast<hype::status::code_t>(__VA_ARGS__) \
        }; \
        if (!__status) { \
            const auto __error = x86::vmx::vm_instruction_error(); \
            TRACE_ERROR("ErrorStatus{category=0x%x code=0x%x, error=0x%x}", __status.category(), __status.code(), static_cast<uint16_t>(__error)); \
            return __status; \
        } \
    } while(0)

} // namespace status

using status_t = status::status_t;

}
