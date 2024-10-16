#include <Nova/core/application.h>
#include <Nova/core/exception.h>
#include <Nova/graphics/renderer.h>
#include <Nova/dotnet/host.h>
#include <stdio.h>

static void ExceptionHandler(const char *message)
{
    MessageBoxA(
        NULL,
        message,
        "NovaEditor Error",
        MB_OK | MB_ICONERROR);
    exit(1);
}

int NvApplicationRun(const NvApplication *app)
{
    NvExceptionSetHandler(ExceptionHandler);

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

    const NvDotnetSettings hostSettings = {
        .dotnetVersion = L"8",
        .runtimePropertiesFilepath = L"./NovaHost.runtimeconfig.json",
        .managedAssemblyFilepath = L"./NovaHost.dll",
    };
    _NvDotnetHostInitialize(&hostSettings);

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
