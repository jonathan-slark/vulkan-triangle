#include <assert.h>
#include <vulkan/vulkan.h>
#include "util.h"
#include "vulkan_device.h"
#include "vulkan_instance.h"
#include "vulkan_physicaldevice.h"

VkDevice device;
VkQueue graphicsQueue;
VkQueue presentQueue;

void createLogicalDevice() {
    struct QueueFamilyIndices indices = findQueueFamilies(getPhysicalDevice());
    float queuePriority = 1.0f;

    // Create a queue for each queue family
    VkDeviceQueueCreateInfo queueCreateInfos[indices.num] = {};
    for (uint32_t i = 0; i < indices.num; i++) {
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
    createInfo.queueCreateInfoCount = indices.num;
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = 0;
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
