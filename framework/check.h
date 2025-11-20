#pragma once


#define verify(...)                     \
    ({                                  \
        auto __result = (__VA_ARGS__);  \
        if (!__result) {                \
            return framework::err(__result.release_error()); \
        }                               \
                                        \
        __result.release_value();       \
    })

#define verify_status(...)              \
    do {                                \
        auto __status = __VA_ARGS__;    \
        if (!__status) {                \
            return __status;            \
        }                               \
    } while (false)


#define trace_status(__status, _msg) trace_error("Status{cat=0x%x, code=0x%x}: " _msg, __status.category(), __status.code())

#define trace_result(...)               \
    do {                                \
        auto __status = framework::status(__VA_ARGS__);    \
        if (!__status) {                \
            trace_status(__status)      \
        }                               \
    } while (false)


#define verify_alloc(_ptr) \
    do {                                \
        if (!_ptr) {                    \
            return framework::err(framework::status(framework::status_category_framework, framework::status_bad_alloc)); \
        }                               \
    } while (false)


#define assert(_cond, _msg) \
    do {                                \
        const auto _res = (_cond);      \
        if (!_res) {                    \
            trace_error("Assert failed: " _msg); \
            return framework::err(framework::status(framework::status_category_framework, framework::status_assert_failed)); \
        }                               \
    } while (false)


#define abort(_msg) \
    do {        \
        trace_error("abort: " _msg); \
        framework::do_abort();     \
    } while (false);
