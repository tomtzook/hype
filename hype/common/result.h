#pragma once

#include "hype_defs.h"
#include "debug.h"


namespace hype::result {

enum category_t {
    HYPE,
    EFI
};

using code_t = uint64_t;

enum hype_code_t : code_t {
    SUCCESS = 0,
    ALLOCATION_ERROR,
    UNSUPPORTED_ERROR,
    ASSERTION_ERROR
};

class status {
public:
    status() noexcept;
    status(code_t code) noexcept;
    status(category_t category, code_t code) noexcept;
    status(status&& other) noexcept;

    category_t category() const noexcept;
    code_t code() const noexcept;

    bool success() const noexcept;

    explicit operator bool() const noexcept;

    status& operator=(const status& other) noexcept;
    status& operator=(status&& other) noexcept;

private:
    category_t m_category;
    code_t m_code;
};

status hype_status(hype_code_t code) noexcept;

}

#define TRACE_STATUS(__status) \
    TRACE_ERROR("Error in call category=0x%x code=0x%x", __status.category(), __status.code());                           \

#define CHECK_AND_JUMP(status, ...) \
    do {            \
        status = hype::result::status {__VA_ARGS__}; \
        if (!status) { \
            TRACE_STATUS(status); \
            goto cleanup; \
        } \
    } while(0)

#define CHECK_AND_RETURN(...) \
    do {            \
        auto __status = hype::result::status {__VA_ARGS__}; \
        if (!__status) { \
            TRACE_STATUS(__status); \
            return __status; \
        } \
    } while(0)


#define CHECK_ASSERT(assertion, message) \
    do {                           \
        if (!(assertion)) {          \
            TRACE_ERROR("Assertion Error: %s", L"" message); \
            CHECK_AND_RETURN(hype::result::ASSERTION_ERROR); \
        } \
    } while(0)

#define CHECK_ALLOCATION(__variable, message) \
    do {                           \
        if (nullptr == __variable) {          \
            TRACE_ERROR("Allocation failed: %s", L"" message); \
            CHECK_AND_RETURN(hype::result::ALLOCATION_ERROR); \
        } \
    } while(0)

#define CHECK_ALLOCATION_AND_JUMP(status, __variable, message) \
    do {                           \
        if (nullptr == __variable) {          \
            TRACE_ERROR("Allocation failed: %s", L"" message); \
            CHECK_AND_JUMP(status, hype::result::ALLOCATION_ERROR); \
        } \
    } while(0)
