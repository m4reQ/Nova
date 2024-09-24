#pragma once
#include <Nova/platform/platform.h>

void _NvRendererClearScreen(void);
bool _NvRendererInitialize(const char *appName, const NvWindowData *windowData);
void _NvRendererShutdown();
