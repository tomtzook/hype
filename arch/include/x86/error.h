#pragma once

#include <error.h>


namespace x86::result {

const common::result_category_t CATEGORY = 3;

enum code_t {
    SUCCESS = 0,
    STATE_NOT_SUPPORTED
};

#ifdef _DEBUG
namespace debug {
const wchar_t* to_string(const common::result& result) noexcept;
}
#endif

}

template<>
common::result_t::result_t(x86::result::code_t code) noexcept;
