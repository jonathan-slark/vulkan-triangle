#include <assert.h>
#include <vulkan/vulkan.h>
#include "util.h"
#include "vulkan_device.h"
#include "vulkan_instance.h"
#include "vulkan_physicaldevice.h"

VkDevice device;
VkQueue graphicsQueue;

void createLogicalDevice() {
    struct QueueFamilyIndices indices = findQueueFamilies(getPhysicalDevice());
    float queuePriority = 1.0f;

    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = indices.graphicsFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures deviceFeatures = {};
    
    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pEnabledFeatures = &deviceFeatures;

#ifdef DEBUG
    createInfo.enabledLayerCount = getNumLayers();
    createInfo.ppEnabledLayerNames = getValidationLayers();
#else
    createInfo.enabledExtensionCount = 0;
#endif // DEBUG

    assert(vkCreateDevice(getPhysicalDevice(), &createInfo, NULL, &device) == VK_SUCCESS);

    vkGetDeviceQueue(device, indices.graphicsFamily, 0, &graphicsQueue);
}

void destroyLogicalDevice() {
    vkDestroyDevice(device, NULL);
}
