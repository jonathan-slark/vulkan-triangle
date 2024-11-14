#ifndef VULKAN_PHYSICALDEVICE_H
#define VULKAN_PHYSICALDEVICE_H

#include <stdbool.h>
#include <vulkan/vulkan.h>

struct QueueFamilyIndices {
    uint32_t graphicsFamily;
    uint32_t presentFamily;
    uint32_t numFamilies;
    bool isSuitable;
};

void pickPhysicalDevice();
VkPhysicalDevice getPhysicalDevice();
struct QueueFamilyIndices findQueueFamilies(VkPhysicalDevice);

#endif // VULKAN_PHYSICALDEVICE_H
