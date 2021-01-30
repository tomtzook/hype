#pragma once

#include <error.h>


namespace x86::result {

constexpr common::result_category_t CATEGORY = 3;

enum code_t {
    INSTRUCTION_FAILED = 1
};

}

template<>
common::result_t::result_t(x86::result::code_t code) noexcept;

#ifdef _DEBUG
namespace x86::result::debug {
const wchar_t* to_string(const common::result& result) noexcept;
}
#endif

