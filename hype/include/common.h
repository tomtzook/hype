#pragma once

#include <crt.h>
#include <debug.h>
#include <error.h>


namespace hype::result {

const common::result_category_t CATEGORY = 0;

enum code_t {
    SUCCESS = 0,
    ALLOCATION_ERROR,
    NOT_SUPPORTED,
    ALREADY_INITIALIZED
};

#ifdef _DEBUG
namespace debug {
const wchar_t* to_string(const common::result& result) noexcept;
}
#endif

}

template<>
common::result::result(hype::result::code_t code) noexcept;
