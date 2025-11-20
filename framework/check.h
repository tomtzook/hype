#pragma once


#define verify(...)                     \
    ({                                  \
        auto __result = (__VA_ARGS__);  \
        if (!__result) {                \
            return __result;            \
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
    } while (false);


#define verify_alloc(_ptr) \
    do {                                \
        if (!_ptr) {                    \
            return framework::status(framework::status_category_framework, framework::status_bad_alloc);            \
        }                               \
    } while (false);
