#pragma once
#include <Nova/core/build.h>
#include <Nova/graphics/window.h>

typedef struct
{
    bool (*onLoad)(void);
    void (*onClose)(void);
    void (*onUpdate)(double frametime);
    NvWindowSettings windowSettings;
    const char *name;
} NvApplication;

NV_API int NvApplicationRun(const NvApplication *app);
