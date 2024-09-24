#pragma once
#include <Nova/core/build.h>
#include <Nova/platform/platform.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    NV_WND_FLAGS_DEFAULT = 0,
    NV_WND_FLAGS_RESIZABLE = 1,
    NV_WND_FLAGS_BORDERLESS = 2,
    NV_WND_FLAGS_TRANSPARENT = 4,
    NV_WND_FLAGS_FULLSCREEN = 8,
    NV_WND_FLAGS_ALLOW_FILE_DROP = 16,
} NvWindowFlags;

typedef struct
{
    uint32_t width, height;
    const wchar_t *title;
    const wchar_t *iconFilepath;
    NvWindowFlags windowFlags;
} NvWindowSettings;

NV_API void NvWindowResize(uint32_t width, uint32_t height);
NV_API void NvWindowMove(uint32_t x, uint32_t y);
NV_API void NvWindowSetTitle(const wchar_t *title);
NV_API void NvWindowClose(void);
NV_API void NvWindowSetIcon(const wchar_t *filepath);
NV_API void NvWindowSetFullscreen(bool isEnabled);
NV_API void NvWindowMaximize(void);
NV_API void NvWindowIconfiy(void);
NV_API void NvWindowGetSize(uint32_t *width, uint32_t *height);
NV_API void NvWindowGetFramebufferSize(uint32_t *width, uint32_t *height);
NV_API void NvWindowGetPosition(uint32_t *x, uint32_t *y);
NV_API bool NvWindowIsVisible(void);

bool _NvWindowInitialize(const NvWindowSettings *settings);
void _NvWindowShutdown(void);
void _NvWindowUpdate(void);
bool _NvWindowShouldClose(void);
const NvWindowData *_NvWindowGetPlatformData(void);
