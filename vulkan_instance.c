#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <vulkan/vulkan.h>
#include "vulkan_instance.h"

#define APPNAME         "Vulkan Test"
#define APPVER          VK_MAKE_VERSION(1, 0, 0)
#define VALIDATIONLAYER "VK_LAYER_KHRONOS_validation"

static const char *layers[] = { VALIDATIONLAYER };
// TODO: Debug log

static VkInstance instance;

static void createInstance();
static void destroyInstance();
static bool checkValidationLayerSupport();

void initVulkan() {
#ifdef DEBUG
    assert(checkValidationLayerSupport());
#endif // DEBUG
    createInstance();
}

void termVulkan() {
    destroyInstance();
}

void createInstance() {
    VkApplicationInfo appInfo = {};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = APPNAME;
    appInfo.applicationVersion = APPVER;
    appInfo.apiVersion         = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
#ifdef DEBUG
    createInfo.enabledLayerCount = 1;
    createInfo.ppEnabledLayerNames = layers;
#endif // DEBUG

    assert(vkCreateInstance(&createInfo, NULL, &instance) == VK_SUCCESS);
}

void destroyInstance() {
    vkDestroyInstance(instance, NULL);
}

bool checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);
    assert(layerCount > 0);

    VkLayerProperties layersAvailable[layerCount];
    vkEnumerateInstanceLayerProperties(&layerCount, layersAvailable);

    for(uint32_t i = 0; i < layerCount; i++) {
	if(strcmp(layersAvailable[i].layerName, VALIDATIONLAYER) == 0 ) {
	    return true;
	}
    }
    return false;
}
