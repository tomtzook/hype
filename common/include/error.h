#pragma once

#include "types.h"
#include "debug.h"


#define CHECK(...) \
    do {            \
        status = __VA_ARGS__; \
        if (!status) { \
            TRACE_ERROR("Error in call (%d): %s", status.code(), common::debug::to_string(status)); \
            goto cleanup; \
        } \
    } while(0)

#define VERIFY_ALLOCATION(mem) \
    do {            \
        if (nullptr == mem) { \
            TRACE_ERROR("Memory is null (unallocated)"); \
            status = common::result::ALLOCATION_ERROR; \
            goto cleanup; \
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
        ALLOCATION_ERROR = 1
    };

    result_t() = delete;
    result_t(result_t&& other) noexcept;
    result_t(result_code_t code, result_category_t category) noexcept;

    template<typename error_type>
    result_t(error_type code) noexcept;

    result_code_t code() const noexcept;
    result_category_t category() const noexcept;

    bool success() const noexcept;
    const wchar_t* message() const noexcept;

    explicit operator bool() const noexcept;
    result_t& operator=(result_t&& other) noexcept;
private:
    result_code_t m_code;
    result_category_t m_category;
};

#ifdef _DEBUG
namespace debug {
const wchar_t* to_string(const result_t& result) noexcept;
const wchar_t* common_error_to_string(const result_t& result) noexcept;
}
#endif

using result = result_t;

}
