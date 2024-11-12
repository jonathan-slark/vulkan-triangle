#include <assert.h>
#include <vulkan/vulkan.h>
#include "vulkan_debug.h"
#include "vulkan_device.h"
#include "vulkan_instance.h"

#define APPNAME "Vulkan Test"
#define APPVER  VK_MAKE_VERSION(1, 0, 0)

const char *validationlayers[] = { VALIDATIONLAYER };

VkInstance instance;

void createInstance();
void destroyInstance();

void initVulkan() {
#ifdef DEBUG
    assert(checkValidationLayerSupport());
#endif // DEBUG
    createInstance();
    pickPhysicalDevice();
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
    createInfo.ppEnabledLayerNames = validationlayers;
#endif // DEBUG

    assert(vkCreateInstance(&createInfo, NULL, &instance) == VK_SUCCESS);
}

void destroyInstance() {
    vkDestroyInstance(instance, NULL);
}
