
#include "x86/error.h"


template<>
common::result::result_t(x86::result::code_t code) noexcept
: m_code(code)
, m_category(x86::result::CATEGORY)
{}

#ifdef _DEBUG
const wchar_t* x86::result::debug::to_string(const common::result& result) noexcept {
    switch (result.code()) {
        case x86::result::SUCCESS: return L"SUCCESS";
        case x86::result::STATE_NOT_SUPPORTED: return L"STATE_NOT_SUPPORTED";
        default: return L"";
    }
}
#endif
