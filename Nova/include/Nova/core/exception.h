#pragma once
#include <Nova/core/build.h>
#include <stdbool.h>

typedef void (*NvExceptionHandler)(const char *message);

NV_API void NvExceptionSetHandler(NvExceptionHandler handler);
NV_API NV_NORETURN void NvExceptionThrow(const char *message);
NV_API bool NvExceptionIsSet(void);
