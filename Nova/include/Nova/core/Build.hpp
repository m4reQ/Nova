#pragma once
#include <stdexcept>
#include <cassert>

#ifdef NV_STATICLIB
    #define NV_API
#else
    #ifdef NV_BUILD
        #define NV_API __declspec(dllexport)
    #else
        #define NV_API __declspec(dllimport)
    #endif
#endif

#ifdef NV_DEBUG
    #define NV_CHECK(cond, msg)                \
        do                                     \
        {                                      \
            if (!(cond))                       \
                throw std::runtime_error(msg); \
        } while (0)
    #define NV_DEBUG_FAIL(msg) assert(false && (msg));
#else
    #define NV_CHECK(cond, msg)
    #define NV_DEBUG_FAIL(msg)
#endif

#define NV_UNUSED(x) (void)x
#define NV_COUNTOF(arr) (sizeof(arr) / sizeof(arr[0]))
#define NV_ASSERT(cond, msg) assert(cond &&msg)
#define IS_FLAG_SET(val, flag) ((val & flag) == flag)

#if defined(_MSC_VER)
    #define NV_UNREACHABLE __assume(0)
#elif defined(__GNUC__) || defined(__clang__)
    #define NV_UNREACHABLE __builtin_unreachable()
#else
    #define NV_UNREACHABLE
#endif

#if defined(_MSC_VER)
#define NV_NORETURN __declspec(noreturn)
#elif defined(__GNUC__) || defined(__clang__)
#define NV_NORETURN __attribute__((noreturn))
#else
#define NV_NORETURN
#endif
