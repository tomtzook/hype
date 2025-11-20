#pragma once

#include "type_traits.h"
#include "optional.h"
#include "debug.h"
#include "check.h"
#include "status.h"

namespace framework {

template<typename t_>
class ok {
public:
    explicit constexpr ok(t_ value);

    t_ value();

private:
    t_ m_value;
};

template<class t_> requires(!is_trivially_copyable_v<t_>)
class ok<t_> {
public:
    explicit constexpr ok(t_&& value);

    t_&& value();

private:
    t_ m_value;
};

template<>
class ok<void> {
public:
    explicit constexpr ok() = default;
};

template<typename t_>
class err {
public:
    // ReSharper disable once CppNonExplicitConvertingConstructor
    constexpr err(t_ value); // NOLINT(*-explicit-constructor)

    t_ value();

private:
    t_ m_value;
};

template<class t_> requires(!is_trivially_copyable_v<t_>)
class err<t_> {
public:
    // ReSharper disable once CppNonExplicitConvertingConstructor
    constexpr err(t_&& value); // NOLINT(*-explicit-constructor)

    t_&& value();

private:
    t_ m_value;
};

template<>
class err<framework_status_codes> {
public:
    // ReSharper disable once CppNonExplicitConvertingConstructor
    constexpr err(framework_status_codes value); // NOLINT(*-explicit-constructor)

    status&& value();

private:
    status m_value;
};

template<typename value_t_, typename err_t_>
class result_base {
public:
    using value_type = value_t_;
    using ok_type = ok<value_t_>;
    using err_type = err<err_t_>;

    // ReSharper disable once CppNonExplicitConvertingConstructor
    constexpr result_base(ok_type&& value); // NOLINT(*-explicit-constructor)
    // ReSharper disable once CppNonExplicitConvertingConstructor
    constexpr result_base(err_type&& value); // NOLINT(*-explicit-constructor)
    template<typename err_t2_>
    // ReSharper disable once CppNonExplicitConvertingConstructor
    constexpr result_base(err<err_t2_>&& value); // NOLINT(*-explicit-constructor)

    explicit operator bool() const;

    constexpr const value_t_& value() const;
    constexpr const err_t_& error() const;

    constexpr value_t_&& release_value();
    constexpr err_t_&& release_error();

private:
    optional<value_t_> m_value;
    optional<err_t_> m_err;
};

template<typename err_t_>
class result_base<void, err_t_> {
public:
    using value_type = void;
    using ok_type = ok<void>;
    using err_type = err<err_t_>;

    // ReSharper disable once CppNonExplicitConvertingConstructor
    constexpr result_base(); // NOLINT(*-explicit-constructor)
    // ReSharper disable once CppNonExplicitConvertingConstructor
    constexpr result_base(ok_type&& type); // NOLINT(*-explicit-constructor)
    // ReSharper disable once CppNonExplicitConvertingConstructor
    constexpr result_base(err_type&& value); // NOLINT(*-explicit-constructor)
    template<typename err_t2_>
    // ReSharper disable once CppNonExplicitConvertingConstructor
    constexpr result_base(err<err_t2_>&& value); // NOLINT(*-explicit-constructor)

    explicit operator bool() const;

    constexpr const err_t_& error() const;

    constexpr void release_value();
    constexpr err_t_&& release_error();

private:
    optional<err_t_> m_err;
};

template<typename t_>
constexpr ok<t_>::ok(t_ value)
    : m_value(value)
{}

template<typename t_>
t_ ok<t_>::value() {
    return m_value;
}

template<class t_> requires(!is_trivially_copyable_v<t_>)
constexpr ok<t_>::ok(t_&& value)
    : m_value(framework::move(value))
{}

template<class t_> requires(!is_trivially_copyable_v<t_>)
t_&& ok<t_>::value() {
    return framework::move(m_value);
}

template<typename t_>
constexpr err<t_>::err(t_ value)
    : m_value(value)
{}

template<typename t_>
t_ err<t_>::value() {
    return m_value;
}

template<class t_> requires(!is_trivially_copyable_v<t_>)
constexpr err<t_>::err(t_&& value)
    : m_value(framework::move(value))
{}

template<class t_> requires(!is_trivially_copyable_v<t_>)
t_&& err<t_>::value() {
    return framework::move(m_value);
}

constexpr err<framework_status_codes>::err(const framework_status_codes value) : m_value(status_category_framework, value) {}

inline status&& err<framework_status_codes>::value() { return framework::move(m_value); }

template<typename value_t_, typename err_t_>
constexpr result_base<value_t_, err_t_>::result_base(ok_type&& value)
    : m_value(framework::move(value.value()))
    , m_err()
{}

template<typename value_t_, typename err_t_>
constexpr result_base<value_t_, err_t_>::result_base(err_type&& value)
    : m_value()
    , m_err(framework::move(value.value()))
{}

template<typename value_t_, typename err_t_>
template<typename err_t2_>
constexpr result_base<value_t_, err_t_>::result_base(err<err_t2_>&& value)
    : m_value()
    , m_err(framework::move(value.value()))
{}

template<typename value_t_, typename err_t_>
result_base<value_t_, err_t_>::operator bool() const { return m_value.has_value(); }

template<typename value_t_, typename err_t_>
constexpr const value_t_& result_base<value_t_, err_t_>::value() const {
    if (!m_value) { abort(); }
    return m_value.value();
}

template<typename value_t_, typename err_t_>
constexpr const err_t_& result_base<value_t_, err_t_>::error() const {
    if (!m_err) { abort(); }
    return m_err.value();
}

template<typename value_t_, typename err_t_>
constexpr value_t_&& result_base<value_t_, err_t_>::release_value() {
    if (!m_value) { abort(); }
    return framework::move(*m_value);
}

template<typename value_t_, typename err_t_>
constexpr err_t_&& result_base<value_t_, err_t_>::release_error() {
    if (!m_err) { abort(); }
    return framework::move(*m_err);
}

template<typename err_t_>
constexpr result_base<void, err_t_>::result_base()
    : m_err()
{}

template<typename err_t_>
constexpr result_base<void, err_t_>::result_base(ok_type&& type)
    : m_err()
{}

template<typename err_t_>
constexpr result_base<void, err_t_>::result_base(err_type&& value)
    : m_err(framework::move(value.value()))
{}

template<typename err_t_>
template<typename err_t2_>
constexpr result_base<void, err_t_>::result_base(err<err_t2_>&& value)
    : m_err(framework::move(value.value()))
{}

template<typename err_t_>
result_base<void, err_t_>::operator bool() const {
    return !m_err.has_value();
}

template<typename err_t_>
constexpr const err_t_& result_base<void, err_t_>::error() const {
    if (!m_err) { abort(); }
    return m_err.value();
}

template<typename err_t_>
constexpr void result_base<void, err_t_>::release_value() {
    if (m_err) { abort(); }
}

template<typename err_t_>
constexpr err_t_&& result_base<void, err_t_>::release_error() {
    if (!m_err) { abort(); }
    return framework::move(*m_err);
}

template<typename value_t_ = void, typename err_t_ = status>
using result = result_base<value_t_, err_t_>;

}
