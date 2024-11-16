#include <stdlib.h>
#include <vulkan/vulkan.h>
#include "vulkan_swapchain.h"

struct SwapChainSupportDetails details;

struct SwapChainSupportDetails getDetails() {
    return details;
}

void querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
    // Get basic surface capabilities
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
	    &details.capabilities);

    // Get supported surface formats
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface,
	    &details.formatCount, NULL);
    if (details.formatCount > 0) {
	details.formats = (VkSurfaceFormatKHR *) malloc(details.formatCount *
		sizeof(VkSurfaceFormatKHR));
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface,
		&details.formatCount, details.formats);
    }

    // Get supported presentation modes
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
	    &details.presentModeCount, NULL);
    if (details.presentModeCount > 0) {
	details.presentModes =
	    (VkPresentModeKHR *) malloc(details.presentModeCount *
		    sizeof(VkPresentModeKHR));
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
		&details.presentModeCount, details.presentModes);
    }
}

void freeSwapChainSupport() {
    free(details.formats);
    free(details.presentModes);
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat() {
    for (uint32_t i = 0; i < details.formatCount; i++) {
	if (details.formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
		details.formats[i].colorSpace ==
		VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
	    return details.formats[i];
	}
    }

    return details.formats[0];
}

VkPresentModeKHR chooseSwapPresentMode() {
    for (uint32_t i = 0; i < details.presentModeCount; i++) {
        if (details.presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            return details.presentModes[i];
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapExtent() {

}
