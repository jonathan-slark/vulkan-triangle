#include <assert.h>
#include <string.h>
#include <vulkan/vulkan.h>
#include "vulkan_instance.h"
#include "vulkan_physicaldevice.h"
#include "vulkan_surface.h"
#include "vulkan_swapchain.h"

const char *deviceExtensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

#define NUMDEVICEEXTENSIONS (sizeof deviceExtensions / sizeof deviceExtensions[0])

VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

VkPhysicalDevice getPhysicalDevice() {
    return physicalDevice;
}

const char **getDeviceExtensions() {
    return deviceExtensions;
}

uint32_t getNumDeviceExtensions() {
    return NUMDEVICEEXTENSIONS;
}

struct QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
    // Get available queue families
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);
    VkQueueFamilyProperties queueFamilies[queueFamilyCount]; // VLA
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
	    queueFamilies);

    // Check for required queue families
    struct QueueFamilyIndices indices;
    indices.isSuitable = false;
    indices.numFamilies = 0;
    for (uint32_t i = 0; i < queueFamilyCount; i++) {
	// Supports graphics commands
	if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
	    indices.graphicsFamily = i;
	    indices.numFamilies++;

	    // Supports presenting to the surface
	    VkBool32 presentSupport = false;
	    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, getSurface(),
		    &presentSupport);
	    if (presentSupport) {
		indices.presentFamily = i;
		indices.numFamilies++;
		indices.isSuitable = true;
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
    VkExtensionProperties availableExtensions[extensionCount]; // VLA
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount,
	    availableExtensions);

    // Check we have required extensions
    for (uint32_t i = 0; i < NUMDEVICEEXTENSIONS; i++) {
	bool extensionFound = false;

	for (uint32_t j = 0; j < extensionCount; j++) {
	    if (strcmp(deviceExtensions[i],
			availableExtensions[j].extensionName) == 0) {
		extensionFound = true;
		break;
	    }
	}

	if (extensionFound) {
	    continue;
	} else {
	    return false;
	}
    }

    return true;
}

bool isDeviceSuitable(VkPhysicalDevice device) {
    struct QueueFamilyIndices indices = findQueueFamilies(device);
    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
	querySwapChainSupport(device, getSurface());
	struct SwapChainSupportDetails details = getDetails();
	swapChainAdequate = details.formats != NULL &&
	    details.presentModes != NULL;
    }

    return indices.isSuitable && extensionsSupported && swapChainAdequate;
}

void pickPhysicalDevice() {
    // Get available physical devices
    uint32_t deviceCount;
    VkInstance instance = getInstance();
    vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
    assert(deviceCount > 0);
    VkPhysicalDevice devices[deviceCount]; // VLA
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices);

    // Select the first suitable device
    for (uint32_t i = 0; i < deviceCount; i++) {
	if (isDeviceSuitable(devices[i])) {
	    physicalDevice = devices[i];
	    break;
	}
    }

    assert(physicalDevice != VK_NULL_HANDLE);
}
