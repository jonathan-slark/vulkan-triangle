#include <assert.h>
#include <stdlib.h>
#include <windows.h>
#include <vulkan/vulkan.h>
#include "vulkan_device.h"
#include "vulkan_physicaldevice.h"
#include "vulkan_surface.h"
#include "vulkan_swapchain.h"
#include "win32.h"

struct SwapChainSupportDetails details;

VkSwapchainKHR swapChain;
VkImage *swapChainImages;
uint32_t swapChainImageNum;
VkFormat swapChainImageFormat;
VkExtent2D swapChainExtent;

struct SwapChainSupportDetails getDetails() {
    return details;
}

VkImage *getSwapChainImages() {
    return swapChainImages;
}

uint32_t getSwapChainImageNum() {
    return swapChainImageNum;
}

VkFormat getSwapChainImageFormat() {
    return swapChainImageFormat;
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
    VkExtent2D actualExtent;
    RECT rcClient;
    const uint32_t minWidth  = details.capabilities.minImageExtent.width;
    const uint32_t maxWidth  = details.capabilities.maxImageExtent.width;
    const uint32_t minHeight = details.capabilities.minImageExtent.height;
    const uint32_t maxHeight = details.capabilities.maxImageExtent.height;

    // Check if width and height don't match the resolution
    if (details.capabilities.currentExtent.width == UINT32_MAX) {
	GetClientRect(getHwnd(), &rcClient);
	actualExtent.width = rcClient.right;
	actualExtent.height = rcClient.bottom;

	if (actualExtent.width < minWidth) {
	    actualExtent.width = minWidth;
	} else if (actualExtent.width > maxWidth) {
	    actualExtent.width = maxWidth;
	}
	if (actualExtent.height < minHeight) {
	    actualExtent.height = minHeight;
	} else if (actualExtent.height > maxHeight) {
	    actualExtent.height = maxHeight;
	}

	return actualExtent;
    } else {
	return details.capabilities.currentExtent;
    }
}

void createSwapChain() {
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat();
    VkPresentModeKHR presentMode = chooseSwapPresentMode();
    VkExtent2D extent = chooseSwapExtent();

    uint32_t imageCount = details.capabilities.minImageCount + 1;
    const uint32_t maxImageCount = details.capabilities.maxImageCount;
    if (maxImageCount > 0 && imageCount > maxImageCount) {
	imageCount = maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = getSurface();

    // Swap chain images
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // Use exclusive if the queue families are the same
    struct QueueFamilyIndices indices = findQueueFamilies(getPhysicalDevice());
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily,
	indices.presentFamily };
    if (indices.graphicsFamily == indices.presentFamily) {
	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    } else {
	createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
	createInfo.queueFamilyIndexCount = 2;
	createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }

    createInfo.preTransform = details.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    assert(vkCreateSwapchainKHR(getDevice(), &createInfo, NULL, &swapChain) == VK_SUCCESS);
    freeSwapChainSupport();

    vkGetSwapchainImagesKHR(getDevice(), swapChain, &imageCount, NULL);
    swapChainImages = (VkImage *) malloc(imageCount * sizeof(VkImage));
    swapChainImageNum = imageCount;
    vkGetSwapchainImagesKHR(getDevice(), swapChain, &imageCount, swapChainImages);

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

void destroySwapChain() {
    vkDestroySwapchainKHR(getDevice(), swapChain, NULL);
    free(swapChainImages);
}
