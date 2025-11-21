#pragma once

#include "debug.h"
#include "check.h"
#include "heap.h"

namespace framework {

template<typename t_>
struct default_deleter {
    void operator()(const t_* t) const {
        delete t;
    }
};

template<typename t_, typename = default_deleter<t_>>
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
    constexpr t_* operator->();
    constexpr const t_* operator->() const;

    [[nodiscard]] const t_* get() const;
    [[nodiscard]] t_* get();
    void reset();
    [[nodiscard]] t_* release();

    template<typename... args_>
    static result<unique_ptr> create(args_... args);
    template<size_t align_, typename... args_>
    static result<unique_ptr> create(args_... args);

private:
    [[nodiscard]] const t_* _get() const {
        if (m_ptr == nullptr) { abort(); }
        return m_ptr;
    }
    [[nodiscard]] t_* _get() {
        if (m_ptr == nullptr) { abort(); }
        return m_ptr;
    }

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
constexpr t_& unique_ptr<t_, deleter_>::operator*() & { return *_get(); }

template<typename t_, typename deleter_>
constexpr const t_& unique_ptr<t_, deleter_>::operator*() const & { return *_get(); }

template<typename t_, typename deleter_>
constexpr t_&& unique_ptr<t_, deleter_>::operator*() && { return framework::move(_get()); }

template<typename t_, typename deleter_>
constexpr const t_&& unique_ptr<t_, deleter_>::operator*() const && { return framework::move(_get()); }

template<typename t_, typename deleter_>
constexpr t_* unique_ptr<t_, deleter_>::operator->() { return _get(); }

template<typename t_, typename deleter_>
constexpr const t_* unique_ptr<t_, deleter_>::operator->() const { return _get(); }

template<typename t_, typename deleter_>
const t_* unique_ptr<t_, deleter_>::get() const { return _get(); }

template<typename t_, typename deleter_>
t_* unique_ptr<t_, deleter_>::get() { return _get(); }

template<typename t_, typename deleter_>
void unique_ptr<t_, deleter_>::reset() {
    if (m_ptr != nullptr) {
        deleter_()(m_ptr);
        m_ptr = nullptr;
    }
}

template<typename t_, typename deleter_>
t_* unique_ptr<t_, deleter_>::release() {
    auto ptr = _get();
    m_ptr = nullptr;
    return ptr;
}

template<typename t_, typename deleter_>
template<typename... args_>
result<unique_ptr<t_, deleter_>> unique_ptr<t_, deleter_>::create(args_... args) {
    auto ptr = new t_(args...);
    return framework::ok(unique_ptr(ptr));
}

template<typename t_, typename deleter_>
template<size_t align_, typename... args_>
result<unique_ptr<t_, deleter_>> unique_ptr<t_, deleter_>::create(args_... args) {
    auto ptr = new (align_) t_(args...);
    return framework::ok(unique_ptr(ptr));
}

}
