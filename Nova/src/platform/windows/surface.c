#include <Nova/graphics/vulkan/surface.h>
#include <Nova/core/build.h>
#include <stdio.h>

bool _NvSurfaceCreateForWindow(NvSurface *out, VkInstance instance, const NvWindowData *windowData)
{
    NV_ASSERT(out != NULL, "Out surface cannot be NULL.");
    NV_ASSERT(windowData != NULL, "Window data cannot be NULL.");

    const VkWin32SurfaceCreateInfoKHR info = {
        .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .hinstance = windowData->instance,
        .hwnd = windowData->window,
    };

    VkSurfaceKHR surface;
    const VkResult result = vkCreateWin32SurfaceKHR(instance, &info, NULL, &surface);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create Vulkan Win32 surface: %x.\n", result);
        return false;
    }

    *out = (NvSurface){
        .ownerInstance = instance,
        .surface = surface,
    };

    return true;
}

void _NvSurfaceDestroy(NvSurface *surface)
{
    if (surface != NULL && surface->surface != NULL)
        vkDestroySurfaceKHR(surface->ownerInstance, surface->surface, NULL);

    memset(surface, 0, sizeof(NvSurface));
}

const char *_NvSurfaceGetRequiredExtension(void)
{
    return VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
}
