#pragma once

#include <error.h>


namespace x86::result {

constexpr common::result_category_t CATEGORY = 3;

enum code_t {
    SUCCESS = 0,
    STATE_NOT_SUPPORTED,
    INSTRUCTION_FAILED
};

}

template<>
common::result_t::result_t(x86::result::code_t code) noexcept;

DEBUG_NAMESPACE_START(x86::result);
const wchar_t* to_string(const common::result& result) noexcept;
DEBUG_NAMESPACE_END
