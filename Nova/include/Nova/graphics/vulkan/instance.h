#pragma once
#include <stdbool.h>
#include <volk.h>

typedef struct
{
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
} NvInstance;

bool _NvInstanceCreate(NvInstance *out, const char *appName);
void _NvInstanceDestroy(NvInstance *instance);
