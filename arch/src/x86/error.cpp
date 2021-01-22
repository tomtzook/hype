
#include "x86/error.h"


template<>
common::result::result_t(x86::result::code_t code) noexcept
: m_code(code)
, m_category(x86::result::CATEGORY)
{}

DEBUG_DECL(x86::result,
const wchar_t* to_string(const common::result& result) noexcept {
    switch (result.code()) {
        case x86::result::SUCCESS: return L"SUCCESS";
        case x86::result::STATE_NOT_SUPPORTED: return L"STATE_NOT_SUPPORTED";
        case x86::result::INSTRUCTION_FAILED: return L"INSTRUCTION_FAILED";
        default: return L"";
    }
}
)
