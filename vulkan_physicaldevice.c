#include <assert.h>
#include <stdbool.h>
#include <vulkan/vulkan.h>
#include "vulkan_instance.h"
#include "vulkan_physicaldevice.h"
#include "vulkan_surface.h"

VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
const char deviceExtensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

VkPhysicalDevice getPhysicalDevice() {
    return physicalDevice;
}

struct QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
    struct QueueFamilyIndices indices;
    indices.num = 0;

    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);

    VkQueueFamilyProperties queueFamilies[queueFamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

    for (uint32_t i = 0; i < queueFamilyCount; i++) {
	if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
	    indices.graphicsFamily = i;
	    indices.num++;

	    VkBool32 presentSupport = false;
	    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, getSurface(), &presentSupport);
	    if (presentSupport) {
		indices.presentFamily = i;
		indices.num++;
		indices.isComplete = true;
		break;
	    }
	}
    }

    return indices;
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
    // Get available extensions
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, NULL);
    VkExtensionProperties availableExtensions[extensionCount];
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, availableExtensions);

    // Check we have required extensions
    for (uint32_t i = 0; i < extensionCount; i++) {
    }
}

bool isDeviceSuitable(VkPhysicalDevice device) {
    struct QueueFamilyIndices indices = findQueueFamilies(device);

    bool extensionsSupported = checkDeviceExtensionSupport(device);

    return indices.isComplete && extensionsSupported;
}

void pickPhysicalDevice() {
    uint32_t deviceCount;
    VkInstance instance = getInstance();

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
