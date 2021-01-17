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
            status = hype::result::ALLOCATION_ERROR; \
            goto cleanup; \
        } \
    } while(0)

namespace common {

using result_code_t = uint64_t;
using result_category_t = uint16_t;

class result {
public:
    result() = delete;
    result(result&& other) noexcept
            : m_code(other.m_code)
            , m_category(other.m_category)
    {}
    result(result_code_t code, result_category_t category) noexcept
        : m_code(code)
        , m_category(category)
    {}

    template<typename error_type>
    result(error_type code) noexcept;

    result_code_t code() const noexcept {
        return m_code;
    }

    result_category_t category() const noexcept {
        return m_category;
    }

    bool success() const noexcept {
        return 0 == m_code;
    }

    explicit operator bool() const noexcept {
        return success();
    }

    result& operator=(result&& other) noexcept {
        m_code = other.m_code;
        m_category = other.m_category;
        return *this;
    }
private:
    result_code_t m_code;
    result_category_t m_category;
};

#ifdef _DEBUG
namespace debug {

const wchar_t* to_string(const result& result) noexcept;

}
#endif

}
