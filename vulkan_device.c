#include <assert.h>
#include <stdbool.h>
#include <vulkan/vulkan.h>
#include "vulkan_device.h"
#include "vulkan_instance.h"

#define UNUSED(x) (void) (x)

VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

bool isDeviceSuitable(VkPhysicalDevice device);

void pickPhysicalDevice() {
    uint32_t deviceCount;
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

bool isDeviceSuitable(VkPhysicalDevice device) {
    UNUSED(device);

    return true;
}
