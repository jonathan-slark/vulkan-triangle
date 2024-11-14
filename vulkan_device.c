#include <assert.h>
#include <vulkan/vulkan.h>
#include "util.h"
#include "vulkan_device.h"
#include "vulkan_instance.h"
#include "vulkan_physicaldevice.h"

const char *deviceExtensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

#define NUMDEVICEEXT (sizeof deviceExtensions / sizeof deviceExtensions[0])

VkDevice device;
VkQueue graphicsQueue;
VkQueue presentQueue;

void createLogicalDevice() {
    // Create a queue for each queue family
    struct QueueFamilyIndices indices = findQueueFamilies(getPhysicalDevice());
    VkDeviceQueueCreateInfo queueCreateInfos[indices.numFamilies] = {}; // VLA
    float queuePriority = 1.0f;
    for (uint32_t i = 0; i < indices.numFamilies; i++) {
	queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfos[i].queueFamilyIndex = i;
	queueCreateInfos[i].queueCount = 1;
	queueCreateInfos[i].pQueuePriorities = &queuePriority;
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};
    
    // Create the physical device
    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos;
    createInfo.queueCreateInfoCount = indices.numFamilies;
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = NUMDEVICEEXT;
    createInfo.ppEnabledExtensionNames = deviceExtensions;
#ifdef DEBUG
    createInfo.enabledLayerCount = getNumLayers();
    createInfo.ppEnabledLayerNames = getValidationLayers();
#endif // DEBUG
    assert(vkCreateDevice(getPhysicalDevice(), &createInfo, NULL, &device) == VK_SUCCESS);

    // Get the queues
    vkGetDeviceQueue(device, indices.graphicsFamily, 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily, 0, &presentQueue);
}

void destroyLogicalDevice() {
    vkDestroyDevice(device, NULL);
}
