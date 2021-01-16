#pragma once

#include "types.h"


namespace hype {

using result_code_t = uint32_t;

enum class result_category_t {
    HYPE = 0,
    EFI
};

class result {
public:
    enum code_t {
        SUCCESS = 0
    };

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

    explicit operator bool() const noexcept {
        return 0 == m_code;
    }
private:
    result_code_t m_code;
    result_category_t m_category;
};

}
