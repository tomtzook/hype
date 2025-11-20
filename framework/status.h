#pragma once

#include "types.h"

namespace framework {

using status_category_t = uint8_t;
using status_code_t = uint32_t;

static constexpr status_category_t status_category_framework = 1;
static constexpr status_code_t status_success = 0;

enum framework_status_codes : status_code_t {
    status_bad_alloc = 1,
    status_bad_arg,
    status_assert_failed,
    status_unsupported
};

class status {
public:
    constexpr status() = default;
    status(const status&) = default;
    status(status&&) = default;

    constexpr status(const status_category_t category, const status_code_t code)
        : m_cat(category)
        , m_code(code)
    {}

    status& operator=(const status&) = default;
    status& operator=(status&&) = default;

    explicit constexpr operator bool() const { return m_code == status_success; }

    [[nodiscard]] constexpr status_category_t category() const { return m_cat; }
    [[nodiscard]] constexpr status_code_t code() const { return m_code; }

    [[nodiscard]] constexpr bool success() const { return m_code == status_success; }
    [[nodiscard]] constexpr bool error() const { return m_code != status_success; }

private:
    status_category_t m_cat;
    status_code_t m_code;
};

}
