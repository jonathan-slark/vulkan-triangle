#include <assert.h>
#include <stdbool.h>
#include <vulkan/vulkan.h>
#include "vulkan_device.h"
#include "vulkan_instance.h"

VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

struct QueueFamilyIndices {
    uint32_t graphicsFamily;
    bool isComplete;
};

struct QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
    struct QueueFamilyIndices indices;

    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);

    VkQueueFamilyProperties queueFamilies[queueFamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

    for (uint32_t i = 0; i < queueFamilyCount; i++) {
	if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
	    indices.graphicsFamily = i;
	    indices.isComplete = true;
	    break;
	}
    }

    return indices;
}

bool isDeviceSuitable(VkPhysicalDevice device) {
    struct QueueFamilyIndices indices = findQueueFamilies(device);

    return indices.isComplete;
}

void pickPhysicalDevice() {
    uint32_t deviceCount;
    VkInstance instance = *getInstance();

    vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
    assert(deviceCount > 0);

    VkPhysicalDevice devices[deviceCount];
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices);

    for (uint32_t i = 0; i < deviceCount; i++) {
	if (isDeviceSuitable(devices[i])) {
	    physicalDevice = devices[i];
	    break;
	}
    }

    assert(physicalDevice != VK_NULL_HANDLE);
}
