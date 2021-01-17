#pragma once

#include "types.h"


#define CHECK(...) \
    do {            \
        status = __VA_ARGS__; \
        if (!status) { \
            TRACE_ERROR("Error in call %d", status.code()); \
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

using result_code_t = uint32_t;

enum class result_category_t {
    HYPE = 0,
    EFI
};

class result {
public:
    result() = delete;
    result(result_code_t code, result_category_t category = result_category_t::HYPE)
        : m_code(code)
        , m_category(category)
    {}

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
private:
    result_code_t m_code;
    result_category_t m_category;
};

}
