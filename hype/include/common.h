#pragma once

#include <crt.h>
#include <debug.h>
#include <error.h>


namespace hype::result {

constexpr common::result_category_t CATEGORY = 1;

enum code_t {
    SUCCESS = 0,
    VMX_NOT_SUPPORTED,
    ALREADY_INITIALIZED,
    HUGE_PAGES_NOT_SUPPORTED,
    UNSUPPORTED_FEATURE
};

}

template<>
common::result_t::result_t(hype::result::code_t code) noexcept;

DEBUG_NAMESPACE_START(hype::result);
const wchar_t* to_string(const common::result& result) noexcept;
DEBUG_NAMESPACE_END
