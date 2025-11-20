#pragma once

#include "common.h"

namespace framework {

struct nullopt_t {};

template<typename t_>
struct optional_storage {
    using type = t_;

    optional_storage();
    optional_storage(const optional_storage&) = default;
    optional_storage(optional_storage&&) = default;
    ~optional_storage();

    optional_storage& operator=(const optional_storage&) = default;
    optional_storage& operator=(optional_storage&&) = default;

    void reset();

    bool has_value;
    union { char dummy; type value; };
};

template<typename t_> requires(is_trivially_destructible_v<t_>)
struct optional_storage<t_> {
    using type = t_;

    optional_storage();
    optional_storage(const optional_storage&) = default;
    optional_storage(optional_storage&&) = default;
    ~optional_storage() = default;

    optional_storage& operator=(const optional_storage&) = default;
    optional_storage& operator=(optional_storage&&) = default;

    void reset();

    bool has_value;
    union { char dummy; type value; };
};

template<typename t_>
class optional {
public:
    constexpr optional() = default;
    optional(const optional&) = default;
    optional(optional&&) = default;
    
    // ReSharper disable once CppNonExplicitConvertingConstructor
    constexpr optional(nullopt_t); // NOLINT(*-explicit-constructor)
    // ReSharper disable once CppNonExplicitConvertingConstructor
    optional(const t_& t); // NOLINT(*-explicit-constructor)
    // ReSharper disable once CppNonExplicitConvertingConstructor
    optional(t_&& t); // NOLINT(*-explicit-constructor)

    ~optional() = default;

    optional& operator=(const optional&) = default;
    optional& operator=(optional&&) = default;
    optional& operator=(const t_& t);
    optional& operator=(t_&& t);
    optional& operator=(nullopt_t);

    constexpr explicit operator bool() const;

    constexpr t_& operator*() &;
    constexpr const t_& operator*() const &;
    constexpr t_&& operator*() &&;
    constexpr const t_&& operator*() const &&;
    constexpr t_& operator->() &;
    constexpr const t_& operator->() const &;

    [[nodiscard]] constexpr bool has_value() const;
    [[nodiscard]] constexpr t_& value() &;
    [[nodiscard]] constexpr const t_& value() const &;
    [[nodiscard]] constexpr t_&& value() &&;
    [[nodiscard]] constexpr const t_&& value() const &&;

private:
    optional_storage<t_> m_storage;
};

template<typename t_>
optional_storage<t_>::optional_storage()
    : has_value(false)
    , dummy(0)
{}

template<typename t_>
optional_storage<t_>::~optional_storage() {
    reset();
}

template<typename t_>
void optional_storage<t_>::reset() {
    if (has_value) {
        value.~type();
        has_value = false;
    }
}

template<typename t_> requires(is_trivially_destructible_v<t_>)
optional_storage<t_>::optional_storage()
    : has_value(false)
    , dummy(0)
{}

template<typename t_> requires(is_trivially_destructible_v<t_>)
// ReSharper disable once CppMemberFunctionMayBeStatic
void optional_storage<t_>::reset() {}

template<typename t_>
constexpr optional<t_>::optional(nullopt_t)
    : m_storage() {}

template<typename t_>
optional<t_>::optional(const t_& t)
    : m_storage() {
    new (&m_storage.value) t_(t);
    m_storage.has_value = true;
}

template<typename t_>
optional<t_>::optional(t_&& t)
    : m_storage() {
    new (&m_storage.value) t_(framework::move(t));
    m_storage.has_value = true;
}

template<typename t_>
optional<t_>& optional<t_>::operator=(const t_& t) {
    m_storage.reset();
    new (&m_storage.value) t_(t);
    m_storage.has_value = true;
    return *this;
}

template<typename t_>
optional<t_>& optional<t_>::operator=(t_&& t) {
    m_storage.reset();
    new (&m_storage.value) t_(framework::move(t));
    m_storage.has_value = true;
    return *this;
}

template<typename t_>
optional<t_>& optional<t_>::operator=(nullopt_t) {
    m_storage.reset();
    return *this;
}

template<typename t_>
constexpr optional<t_>::operator bool() const { return m_storage.has_value; }

template<typename t_>
constexpr t_& optional<t_>::operator*() & { return m_storage.value; }

template<typename t_>
constexpr const t_& optional<t_>::operator*() const & { return m_storage.value; }

template<typename t_>
constexpr t_&& optional<t_>::operator*() && { return framework::move(m_storage.value); }

template<typename t_>
constexpr const t_&& optional<t_>::operator*() const && { return framework::move(m_storage.value); }

template<typename t_>
constexpr t_& optional<t_>::operator->() & { return m_storage.value; }

template<typename t_>
constexpr const t_& optional<t_>::operator->() const & { return m_storage.value; }

template<typename t_>
constexpr bool optional<t_>::has_value() const { return m_storage.has_value; }

template<typename t_>
constexpr t_& optional<t_>::value() & { return m_storage.value; }

template<typename t_>
constexpr const t_& optional<t_>::value() const & { return m_storage.value; }

template<typename t_>
constexpr t_&& optional<t_>::value() && { return framework::move(m_storage.value); }

template<typename t_>
constexpr const t_&& optional<t_>::value() const && { return framework::move(m_storage.value); }

}
