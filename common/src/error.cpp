
#include "error.h"


common::result_t::result_t() noexcept
    : m_code(common_code_t::SUCCESS)
    , m_category(COMMON)
{}
common::result_t::result_t(result_t&& other) noexcept
    : m_code(other.m_code)
    , m_category(other.m_category)
{}
common::result_t::result_t(result_code_t code, result_category_t category) noexcept
    : m_code(code)
    , m_category(category)
{}

common::result_code_t common::result_t::code() const noexcept {
    return m_code;
}

common::result_category_t common::result_t::category() const noexcept {
    return m_category;
}

bool common::result_t::success() const noexcept {
    return 0 == m_code;
}

const wchar_t* common::result_t::message() const noexcept {
#ifdef _DEBUG
    return common::debug::to_string(*this);
#else
    return L"";
#endif
}

common::result_t::operator bool() const noexcept {
    return success();
}

common::result_t& common::result_t::operator=(const result_t& other) noexcept {
    m_code = other.m_code;
    m_category = other.m_category;
    return *this;
}

common::result_t& common::result_t::operator=(result_t&& other) noexcept {
    m_code = other.m_code;
    m_category = other.m_category;
    return *this;
}

template<>
common::result_t::result_t(common::result_t::common_code_t code) noexcept
    : m_code(code)
    , m_category(COMMON)
{}

DEBUG_DECL(common,
const wchar_t* common_error_to_string(const common::result& result) noexcept {
    switch (result.code()) {
        case common::result::SUCCESS: return L"SUCCESS";
        case common::result::ALLOCATION_ERROR: return L"ALLOCATION_ERROR";
        case common::result::ASSERTION_ERROR: return L"ASSERTION_ERROR";
        default: return L"";
    }
}
)
