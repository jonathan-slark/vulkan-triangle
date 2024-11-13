#include <assert.h>
#include <vulkan/vulkan.h>
#include "config.h"
#include "vulkan_debug.h"
#include "vulkan_device.h"
#include "vulkan_physicaldevice.h"
#include "vulkan_instance.h"

// Based on:
// https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/00_Base_code.html

const char *validationLayers[] = { VALIDATIONLAYER };
#ifdef DEBUG
const char *extensions[]       = { DEBUGEXT };
#else
const char *extensions[];
#endif // DEBUG

#define NUMLAYERS (sizeof validationLayers / sizeof validationLayers[0])
#define NUMEXT    (sizeof extensions / sizeof extensions[0])

VkInstance instance;

VkInstance getInstance() {
    return instance;
}

const char **getValidationLayers() {
    return validationLayers;
}

int getNumLayers() {
    return NUMLAYERS;
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
    createInfo.enabledLayerCount = NUMLAYERS;
    createInfo.ppEnabledLayerNames = validationLayers;
    createInfo.enabledExtensionCount = NUMEXT;
    createInfo.ppEnabledExtensionNames = extensions;
#endif // DEBUG

    assert(vkCreateInstance(&createInfo, NULL, &instance) == VK_SUCCESS);
}

void destroyInstance() {
    vkDestroyInstance(instance, NULL);
}

void initVulkan() {
#ifdef DEBUG
    assert(checkValidationLayerSupport());
#endif // DEBUG
    createInstance();
#ifdef DEBUG
    createDebugMessenger();
#endif
    pickPhysicalDevice();
    createLogicalDevice();
}

void termVulkan() {
    destroyLogicalDevice();
#ifdef DEBUG
    destroyDebugMessenger();
#endif
    destroyInstance();
}
