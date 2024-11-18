#ifndef VULKAN_DEVICE_H
#define VULKAN_DEVICE_H

#include <vulkan/vulkan.h>

VkDevice getDevice();
void createLogicalDevice();
void destroyLogicalDevice();

#endif // VULKAN_DEVICE_H
