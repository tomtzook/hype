#pragma once

#include "types.h"
#include "debug.h"


#define ERROR(result) \
    do {              \
        status = result; \
        goto cleanup; \
    } while(0)

#define CHECK(...) \
    do {            \
        status = __VA_ARGS__; \
        if (!status) { \
            TRACE_ERROR("Error in call (%d): %s", status.code(), common::debug::to_string(status)); \
            goto cleanup; \
        } \
    } while(0)

#define CHECK_ALLOCATION(mem) \
    do {            \
        if (nullptr == mem) { \
            TRACE_ERROR("Memory is null (unallocated)"); \
            status = common::result::ALLOCATION_ERROR; \
            goto cleanup; \
        } \
    } while(0)

#define CHECK_ASSERT(assertion, message) \
    do {                           \
        if (!(assertion)) {          \
            TRACE_ERROR("Assertion Error: %s", L"" message); \
            status = common::result::ASSERTION_ERROR;        \
            goto cleanup; \
        } \
    } while(0)

#define CHECK_SILENT(...) \
    do {            \
        common::result __status = __VA_ARGS__; \
        if (!__status) { \
            TRACE_ERROR("Error in call (%d): %s", __status.code(), common::debug::to_string(__status)); \
        } \
    } while(0)

namespace common {

using result_code_t = uint64_t;
using result_category_t = uint16_t;

class result_t {
public:
    static const result_category_t COMMON = 0;
    enum common_code_t {
        SUCCESS = 0,
        ALLOCATION_ERROR,
        ASSERTION_ERROR,
        UNSUPPORTED_ERROR
    };

    result_t() noexcept;
    result_t(result_t&& other) noexcept;
    result_t(result_code_t code, result_category_t category) noexcept;

    template<typename error_type>
    result_t(error_type code) noexcept;

    result_code_t code() const noexcept;
    result_category_t category() const noexcept;

    bool success() const noexcept;
    const wchar_t* message() const noexcept;

    explicit operator bool() const noexcept;

    result_t& operator=(const result_t& other) noexcept;
    result_t& operator=(result_t&& other) noexcept;
private:
    result_code_t m_code;
    result_category_t m_category;
};

using result = result_t;

}

template<>
common::result_t::result_t(common::result_t::common_code_t code) noexcept;


#ifdef _DEBUG
namespace common::debug {
const wchar_t* to_string(const result_t& result) noexcept;
const wchar_t* common_error_to_string(const result_t& result) noexcept;
}
#endif
