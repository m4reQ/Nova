#pragma once
#include <Nova/core/build.h>
#include <stdint.h>

NV_API uint64_t NvTimeUtcGetNow(void);
NV_API uint64_t NvTimeLocalGetNow(void);
NV_API uint64_t NvTimePerformanceGetNow(void);
NV_API uint64_t NvTimePerformanceGetResolution(void);
