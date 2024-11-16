#ifndef VULKAN_SWAPCHAIN_H
#define VULKAN_SWAPCHAIN_H

#include <vulkan/vulkan.h>

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR *formats;
    VkPresentModeKHR *presentModes;
    uint32_t formatCount;
    uint32_t presentModeCount;
};

struct SwapChainSupportDetails getDetails();
void querySwapChainSupport(VkPhysicalDevice, VkSurfaceKHR);
void freeSwapChainSupport();

#endif // VULKAN_SWAPCHAIN_H
