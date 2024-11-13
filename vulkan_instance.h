#ifndef VULKAN_INSTANCE_H
#define VULKAN_INSTANCE_H

#include <vulkan/vulkan.h>

#define VK_USE_PLATFORM_WIN32_KHR

VkInstance getInstance();
const char **getValidationLayers();
int getNumLayers();
void initVulkan();
void termVulkan();

#endif // VULKAN_INSTANCE_H
