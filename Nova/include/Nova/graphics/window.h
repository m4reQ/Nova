#pragma once
#include <Nova/core/build.h>
#include <Nova/platform/platform.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    NV_WND_FLAGS_DEFAULT = 0,
} NvWindowFlags;

typedef struct
{
    uint32_t width, height;
    const wchar_t *title;
    NvWindowFlags windowFlags;
} NvWindowSettings;

NV_API NvWindowResize(uint32_t width, uint32_t height);
NV_API NvWindowMove(uint32_t x, uint32_t y);
NV_API NvWindowSetTitle(const wchar_t *title);
NV_API NvWindowClose(void);

bool _NvWindowInitialize(const NvWindowSettings *settings, const NvStartupData *startupData);
void _NvWindowShutdown(void);
void _NvWindowUpdate(void);
bool _NvWindowShouldClose(void);
const NvWindowData *_NvWindowGetPlatformData(void);
