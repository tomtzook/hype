#pragma once

#include <crt.h>
#include <debug.h>
#include <error.h>


namespace hype::result {

const common::result_category_t CATEGORY = 1;

enum code_t {
    SUCCESS = 0,
    VMX_NOT_SUPPORTED,
    ALREADY_INITIALIZED
};

#ifdef _DEBUG
namespace debug {
const wchar_t* to_string(const common::result& result) noexcept;
}
#endif

}

template<>
common::result_t::result_t(hype::result::code_t code) noexcept;
