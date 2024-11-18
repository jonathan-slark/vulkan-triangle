#include <assert.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>
#include "vulkan_device.h"
#include "vulkan_imageview.h"
#include "vulkan_swapchain.h"

VkImageView *swapChainImageViews;

void createImageViews() {
    VkImage *swapChainImages = getSwapChainImages();
    uint32_t swapChainImageNum = getSwapChainImageNum();

    swapChainImageViews = (VkImageView *) malloc(swapChainImageNum *
	    sizeof(VkImageView));

    for (uint32_t i = 0; i < swapChainImageNum; i++) {
	VkImageViewCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.image = swapChainImages[i];
	createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
	createInfo.subresourceRange.baseMipLevel   = 0;
	createInfo.subresourceRange.levelCount     = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount     = 1;

	assert(vkCreateImageView(getDevice(), &createInfo, NULL,
		    &swapChainImageViews[i]) == VK_SUCCESS);
    }
}

void destroyImageViews() {
    uint32_t swapChainImageNum = getSwapChainImageNum();

    for (uint32_t i = 0; i < swapChainImageNum; i++) {
	vkDestroyImageView(getDevice(), swapChainImageViews[i], NULL);
    }

    free(swapChainImageViews);
}
