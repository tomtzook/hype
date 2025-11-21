#pragma once

namespace framework {

template<class _t, _t _v>
struct integral_constant {
    using value_type = _t;
    using type = integral_constant;
    static constexpr _t value = _v;

    // ReSharper disable once CppNonExplicitConversionOperator
    constexpr operator value_type() const { return value; } // NOLINT(*-explicit-constructor)
    constexpr value_type operator()() const { return value; }
};

using true_type = integral_constant<bool, true>;
using false_type = integral_constant<bool, false>;

template<class _t, class... _args>
struct is_trivially_constructible
    : integral_constant<bool, __is_trivially_constructible(_t, _args...)> {};

template<class _t, class... _args>
inline constexpr bool is_trivially_constructible_v = __is_trivially_constructible(_t, _args...);

template<class _t>
struct is_trivially_copyable : integral_constant<bool, __is_trivially_copyable(_t)> {};

template<class _t>
inline constexpr bool is_trivially_copyable_v = __is_trivially_copyable(_t);

template<class _t>
struct is_trivially_destructible : integral_constant<bool, __is_trivially_destructible(_t)> {};

template<class _t>
inline constexpr bool is_trivially_destructible_v = __is_trivially_destructible(_t);

template<bool, typename = void>
struct enable_if {
};

template<typename _t>
struct enable_if<true, _t> {
    using type = _t;
};

template<bool _cond, typename _t = void>
using enable_if_t = enable_if<_cond, _t>::type;

template<class _t>
struct remove_reference {
    using type = _t;
};

template<class _t>
struct remove_reference<_t&> {
    using type = _t;
};

template<class _t>
struct remove_reference<_t&&> {
    using type = _t;
};

template <typename t_, typename u_>
inline constexpr bool is_same_v = __is_same(t_, u_);

template<typename _t, typename _t2>
constexpr bool is_any_of(_t first, _t2 second) {
    return first == second;
}

template<typename _t, typename _t2, typename... _args>
constexpr bool is_any_of(_t first, _t2 second, _args... args) {
    return first == second || is_any_of(first, args...);
}

template<typename _t>
constexpr remove_reference<_t>::type&& move(_t&& arg) {
    return static_cast<remove_reference<_t>::type&&>(arg);
}



}
