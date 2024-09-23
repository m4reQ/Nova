#pragma once
#include <assert.h>

#ifdef NV_BUILD
#define NV_API __declspec(dllexport)
#else
#define NV_API __declspec(dllimport)
#endif

#define NV_ASSERT(cond, msg) assert(cond &&msg)
