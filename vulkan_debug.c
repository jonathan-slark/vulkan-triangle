#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <vulkan/vulkan.h>
#include "vulkan_debug.h"

// TODO: Debug log

bool checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);
    if (layerCount == 0) {
	return false;
    }

    VkLayerProperties layersAvailable[layerCount];
    vkEnumerateInstanceLayerProperties(&layerCount, layersAvailable);

    for (uint32_t i = 0; i < layerCount; i++) {
	if (strcmp(layersAvailable[i].layerName, VALIDATIONLAYER) == 0) {
	    return true;
	}
    }
    return false;
}
