
#include "status.h"


namespace hype::status {

status_t::status_t() noexcept
        : status_t(hype_code_t::success) {
}

status_t::status_t(category_t category, code_t code) noexcept
        : m_category(category)
        , m_code(code) {
}

status_t::status_t(hype_code_t code) noexcept
    : m_category(category_t::hype)
    , m_code(code) {

}

status_t::status_t(status_t&& other) noexcept
        : m_category(other.m_category)
        , m_code(other.m_code) {
}

category_t status_t::category() const noexcept {
    return m_category;
}

code_t status_t::code() const noexcept {
    return m_code;
}

bool status_t::success() const noexcept {
    return m_code == hype::status::success;
}

status_t::operator bool() const noexcept {
    return m_code == hype::status::success;
}

status_t& status_t::operator=(const status_t& other) noexcept {
    m_category = other.m_category;
    m_code = other.m_code;

    return *this;
}

status_t& status_t::operator=(status_t&& other) noexcept {
    m_category = other.m_category;
    m_code = other.m_code;

    return *this;
}

}

