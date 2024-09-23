#include <Nova/core/application.h>
#include <stdio.h>

int NvApplicationRun(const NvApplication *app)
{
    if (!_NvWindowInitialize(&app->windowSettings, NULL))
    {
        fprintf(stderr, "Failed to initialize window.\n");
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
