#pragma once
#include <Nova/platform/platform.h>
#include <stdbool.h>
#include <volk.h>

typedef struct
{
    VkInstance ownerInstance;
    VkSurfaceKHR surface;
} NvSurface;

bool _NvSurfaceCreateForWindow(NvSurface *out, VkInstance instance, const NvWindowData *windowData);
void _NvSurfaceDestroy(NvSurface *surface);
const char *_NvSurfaceGetRequiredExtension(void);
