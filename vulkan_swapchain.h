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
VkImage *getSwapChainImages();
uint32_t getSwapChainImageNum();
void querySwapChainSupport(VkPhysicalDevice, VkSurfaceKHR);
void freeSwapChainSupport();
void createSwapChain();
void destroySwapChain();

#endif // VULKAN_SWAPCHAIN_H
