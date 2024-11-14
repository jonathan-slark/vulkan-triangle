#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>
#include "vulkan_instance.h"
#include "vulkan_physicaldevice.h"
#include "vulkan_surface.h"

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR *formats;
    VkPresentModeKHR *presentModes;
} swapChainSupport;

VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

VkPhysicalDevice getPhysicalDevice() {
    return physicalDevice;
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
    VkExtensionProperties availableExtensions[extensionCount];
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount,
	    availableExtensions);

    // Check we have required extensions
    for (uint32_t i = 0; i < extensionCount; i++) {
	// TODO
    }

    return true;
}

void querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
    struct SwapChainSupportDetails swapChainSupport;

    // Get basic surface capabilities
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
	    &swapChainSupport.capabilities);

    // Get supported surface formats
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, NULL);
    if (formatCount > 0) {
	swapChainSupport.formats = calloc(formatCount,
		sizeof(VkSurfaceFormatKHR));
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount,
		swapChainSupport.formats);
    }

    // Get supported presentation modes
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
	    &presentModeCount, NULL);
    if (presentModeCount != 0) {
	swapChainSupport.presentModes = calloc(presentModeCount,
		sizeof(VkPresentModeKHR));
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
		&presentModeCount, swapChainSupport.presentModes);
    }
}

void freeSwapChainSupport() {
    free(swapChainSupport.formats);
    free(swapChainSupport.presentModes);
}

bool isDeviceSuitable(VkPhysicalDevice device) {
    struct QueueFamilyIndices indices = findQueueFamilies(device);
    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
	querySwapChainSupport(device, getSurface());
	swapChainAdequate =
	    swapChainSupport.formats      != NULL &&
	    swapChainSupport.presentModes != NULL;
	freeSwapChainSupport();
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
