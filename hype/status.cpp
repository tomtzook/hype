
#include "status.h"


namespace hype::status {

status_t::status_t()
        : status_t(hype_code_t::success) {
}

status_t::status_t(category_t category, code_t code)
        : m_category(category)
        , m_code(code) {
}

status_t::status_t(hype_code_t code)
    : m_category(category_t::hype)
    , m_code(code) {

}

status_t::status_t(status_t&& other)
        : m_category(other.m_category)
        , m_code(other.m_code) {
}

category_t status_t::category() const {
    return m_category;
}

code_t status_t::code() const {
    return m_code;
}

bool status_t::success() const {
    return m_code == hype::status::success;
}

status_t::operator bool() const {
    return m_code == hype::status::success;
}

status_t& status_t::operator=(const status_t& other) {
    m_category = other.m_category;
    m_code = other.m_code;

    return *this;
}

status_t& status_t::operator=(status_t&& other) {
    m_category = other.m_category;
    m_code = other.m_code;

    return *this;
}

}

