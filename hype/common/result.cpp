
#include "result.h"


namespace hype::result {

status::status() noexcept
    : status(SUCCESS) {
}

status::status(code_t code) noexcept
    : status(category_t::HYPE, code) {
}

status::status(category_t category, code_t code) noexcept
    : m_category(category)
    , m_code(code) {
}

status::status(status&& other) noexcept
    : m_category(other.m_category)
    , m_code(other.m_code) {
}

category_t status::category() const noexcept {
    return m_category;
}

code_t status::code() const noexcept {
    return m_code;
}

bool status::success() const noexcept {
    return m_code == SUCCESS;
}

status::operator bool() const noexcept {
    return m_code == SUCCESS;
}

status& status::operator=(const status& other) noexcept {
    m_category = other.m_category;
    m_code = other.m_code;
}

status& status::operator=(status&& other) noexcept {
    m_category = other.m_category;
    m_code = other.m_code;
}

status hype_status(hype_code_t code) noexcept {
    return {code};
}

}
