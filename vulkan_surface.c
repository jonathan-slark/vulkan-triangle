#include <assert.h>
#include <windows.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#include "vulkan_instance.h"
#include "vulkan_surface.h"
#include "win32.h"

VkSurfaceKHR surface;

VkSurfaceKHR getSurface() {
    return surface;
}

void createSurface() {
    VkWin32SurfaceCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = getHwnd();
    createInfo.hinstance = GetModuleHandle(NULL);

    assert(vkCreateWin32SurfaceKHR(getInstance(), &createInfo, NULL, &surface) == VK_SUCCESS);
}

void destroySurface() {
    vkDestroySurfaceKHR(getInstance(), surface, NULL);
}
