#pragma once

#include "heap.h"

namespace framework {

template<typename t_>
struct default_deleter {
    void operator()(const t_* t) const {
        delete t;
    }
};

template<typename t_, typename deleter_ = default_deleter<t_>>
class unique_ptr {
public:
    using type = t_;

    unique_ptr();
    unique_ptr(const unique_ptr&) = delete;
    explicit unique_ptr(t_* t);
    unique_ptr(unique_ptr&&) noexcept;
    ~unique_ptr();

    unique_ptr& operator=(const unique_ptr&) = delete;
    unique_ptr& operator=(unique_ptr&&) noexcept;

    constexpr explicit operator bool() const;

    constexpr t_& operator*() &;
    constexpr const t_& operator*() const &;
    constexpr t_&& operator*() &&;
    constexpr const t_&& operator*() const &&;
    constexpr t_& operator->() &;
    constexpr const t_& operator->() const &;

    void reset();
    t_* release();

    template<typename... args_>
    static result<unique_ptr> create(args_... args);

private:
    t_* m_ptr;
};

template<typename t_, typename deleter_>
unique_ptr<t_, deleter_>::unique_ptr() : m_ptr(nullptr) {}

template<typename t_, typename deleter_>
unique_ptr<t_, deleter_>::unique_ptr(t_* t) : m_ptr(t) {}

template<typename t_, typename deleter_>
unique_ptr<t_, deleter_>::unique_ptr(unique_ptr&& other) noexcept
    : m_ptr(other.m_ptr) {
    other.m_ptr = nullptr;
}

template<typename t_, typename deleter_>
unique_ptr<t_, deleter_>::~unique_ptr() {
    reset();
}

template<typename t_, typename deleter_>
unique_ptr<t_, deleter_>& unique_ptr<t_, deleter_>::operator=(unique_ptr&& other) noexcept {
    m_ptr = other.m_ptr;
    other.m_ptr = nullptr;
    return *this;
}

template<typename t_, typename deleter_>
constexpr unique_ptr<t_, deleter_>::operator bool() const { return m_ptr != nullptr; }

template<typename t_, typename deleter_>
constexpr t_& unique_ptr<t_, deleter_>::operator*() & { return *m_ptr; }

template<typename t_, typename deleter_>
constexpr const t_& unique_ptr<t_, deleter_>::operator*() const & { return *m_ptr; }

template<typename t_, typename deleter_>
constexpr t_&& unique_ptr<t_, deleter_>::operator*() && { return framework::move(m_ptr); }

template<typename t_, typename deleter_>
constexpr const t_&& unique_ptr<t_, deleter_>::operator*() const && { return framework::move(m_ptr); }

template<typename t_, typename deleter_>
constexpr t_& unique_ptr<t_, deleter_>::operator->() & { return m_ptr; }

template<typename t_, typename deleter_>
constexpr const t_& unique_ptr<t_, deleter_>::operator->() const & { return m_ptr; }

template<typename t_, typename deleter_>
void unique_ptr<t_, deleter_>::reset() {
    if (m_ptr != nullptr) {
        deleter_()(m_ptr);
        m_ptr = nullptr;
    }
}

template<typename t_, typename deleter_>
t_* unique_ptr<t_, deleter_>::release() {
    auto ptr = m_ptr;
    m_ptr = nullptr;
    return ptr;
}

template<typename t_, typename deleter_>
template<typename... args_>
result<unique_ptr<t_, deleter_>> unique_ptr<t_, deleter_>::create(args_... args) {
    auto ptr = new t_(args...);
    return unique_ptr(ptr);
}

}
