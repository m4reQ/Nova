#pragma once
#include <assert.h>

#ifdef NV_BUILD
#define NV_API __declspec(dllexport)
#else
#define NV_API __declspec(dllimport)
#endif

#define NV_ASSERT(cond, msg) assert(cond &&msg)
#define NV_ARRLEN(x) (sizeof(x) / sizeof(x[0]))
#define NV_UNUSED(x) (void)x
#define IS_FLAG_SET(flags, flag) ((flags & flag) == flag)

#if defined(_MSC_VER)
#define NV_UNREACHABLE __assume(0)
#elif defined(__GNUC__) || defined(__clang__)
#define NV_UNREACHABLE __builtin_unreachable()
#else
#define NV_UNREACHABLE
#endif
