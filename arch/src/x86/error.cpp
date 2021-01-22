
#include "x86/error.h"


template<>
common::result::result_t(x86::result::code_t code) noexcept
: m_code(code)
, m_category(x86::result::CATEGORY)
{}
