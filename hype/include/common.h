#pragma once

#include <crt.h>
#include <debug.h>
#include <error.h>


namespace hype::result {

constexpr common::result_category_t CATEGORY = 1;

enum code_t {
    VMX_NOT_SUPPORTED = 1,
    ALREADY_INITIALIZED,
    HUGE_PAGES_NOT_SUPPORTED,
};

}

template<>
common::result_t::result_t(hype::result::code_t code) noexcept;

#ifdef _DEBUG
namespace hype::result::debug {
const wchar_t* to_string(const common::result& result) noexcept;
}
#endif
