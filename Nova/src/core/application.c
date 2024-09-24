#include <Nova/core/application.h>
#include <Nova/graphics/renderer.h>
#include <stdio.h>

int NvApplicationRun(const NvApplication *app)
{
    if (!_NvWindowInitialize(&app->windowSettings))
    {
        fprintf(stderr, "Failed to initialize window.\n");
        return 1;
    }

    if (!_NvRendererInitialize(app->name, _NvWindowGetPlatformData()))
    {
        fprintf(stderr, "Failed to initialize renderer.\n");
        return 1;
    }

    if (!app->onLoad())
    {
        fprintf(stderr, "User loading function failed.\n");
        return 1;
    }

    while (!_NvWindowShouldClose())
    {
        _NvWindowUpdate();
        app->onUpdate(0.0);
    }

    app->onClose();
    _NvWindowShutdown();

    return 0;
}
