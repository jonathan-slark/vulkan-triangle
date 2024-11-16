#ifndef VULKAN_INSTANCE_H
#define VULKAN_INSTANCE_H

#include <vulkan/vulkan.h>

#define VK_USE_PLATFORM_WIN32_KHR

#ifdef DEBUG
const char **getValidationLayers();
uint32_t getNumLayers();
#endif // DEBUG

VkInstance getInstance();
void initVulkan();
void termVulkan();

#endif // VULKAN_INSTANCE_H
