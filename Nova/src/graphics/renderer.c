#include <Nova/graphics/renderer.h>
#include <Nova/graphics/vulkan/surface.h>
#include <Nova/graphics/vulkan/instance.h>
#include <stdio.h>

static NvInstance s_Instance;
static NvSurface s_WindowSurface;

bool _NvRendererInitialize(const char *appName, const NvWindowData *windowData)
{
    bool success = false;

    if (!_NvInstanceCreate(&s_Instance, appName))
        goto end;

    if (!_NvSurfaceCreateForWindow(&s_WindowSurface, s_Instance.instance, windowData))
        goto end;

    success = true;

end:
    if (!success)
        fprintf(stderr, "Failed to initialize renderer.\n");

    return success;
}

void _NvRendererShutdown()
{
    _NvSurfaceDestroy(&s_WindowSurface);
    _NvInstanceDestroy(&s_Instance);
}

void _NvRendererClearScreen(void)
{
}
