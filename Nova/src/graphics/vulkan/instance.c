#include <Nova/graphics/vulkan/instance.h>
#include <Nova/graphics/vulkan/surface.h>
#include <Nova/core/build.h>
#include <stb/stb_ds.h>
#include <stdio.h>
#include <string.h>

#define MESSAGE_SEVERITY VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
#define MESSAGE_TYPE VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT

static VkBool32 VKAPI_PTR DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData)
{
    if (IS_FLAG_SET(messageSeverity, VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT))
        fprintf(stderr, "Vulkan ERROR: %s.\n", pCallbackData->pMessage);
    else
        fprintf(stdout, "Vulkan: %s.\n", pCallbackData->pMessage);

    return VK_TRUE;
}

bool _NvInstanceCreate(NvInstance *out, const char *appName)
{
    NV_ASSERT(out != NULL, "Out instance cannot be NULL");

    bool success = false;
    const char **extensions = NULL;
    const char **layers = NULL;

    const VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .apiVersion = VK_API_VERSION_1_3,
        .applicationVersion = VK_MAKE_API_VERSION(1, 0, 0, 0),
        .engineVersion = VK_MAKE_API_VERSION(1, 0, 0, 0),
        .pApplicationName = appName,
        .pEngineName = "Nova",
    };

    arrput(extensions, VK_KHR_SURFACE_EXTENSION_NAME);
    arrput(extensions, _NvSurfaceGetRequiredExtension());
#if NV_DEBUG
    arrput(extensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

#if NV_DEBUG
    arrput(layers, "VK_LAYER_KHRONOS_validation");
#endif

    VkInstanceCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .ppEnabledExtensionNames = extensions,
        .enabledExtensionCount = arrlen(extensions),
        .ppEnabledLayerNames = layers,
        .enabledLayerCount = arrlen(layers),
    };

#if NV_DEBUG
    VkDebugUtilsMessengerCreateInfoEXT debugInfo = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .pfnUserCallback = DebugCallback,
        .messageSeverity = MESSAGE_SEVERITY,
        .messageType = MESSAGE_TYPE,
    };

    info.pNext = &debugInfo;
#endif

    VkResult result = volkInitialize();
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to load Vulkan bindings: %x.\n", result);
        goto end;
    }

    VkInstance instance;
    result = vkCreateInstance(&info, NULL, &instance);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create Vulkan instance: %x.\n", result);
        goto end;
    }

    volkLoadInstanceOnly(instance);

    VkDebugUtilsMessengerEXT debugMessenger = NULL;
#if NV_DEBUG
    result = vkCreateDebugUtilsMessengerEXT(instance, &debugInfo, NULL, &debugMessenger);
    if (result != VK_SUCCESS)
        fprintf(stderr, "Failed to create Vulkan debug callback: %x\n", result);
#endif

    *out = (NvInstance){
        .debugMessenger = debugMessenger,
        .instance = instance,
    };

    success = true;

end:
    arrfree(extensions);
    arrfree(layers);
    return success;
}

void _NvInstanceDestroy(NvInstance *instance)
{
    if (instance != NULL)
    {
        if (instance->debugMessenger != NULL)
            vkDestroyDebugUtilsMessengerEXT(instance->instance, instance->debugMessenger, NULL);

        if (instance->instance != NULL)
            vkDestroyInstance(instance->instance, NULL);
    }

    memset(instance, 0, sizeof(NvInstance));
}
