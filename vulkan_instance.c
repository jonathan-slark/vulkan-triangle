#include <assert.h>
#include <vulkan/vulkan.h>
#include "vulkan_instance.h"

#define APPNAME "Vulkan Test"

static VkInstance instance;

static void createInstance();

void initVulkan() {
    createInstance();
}

void createInstance() {
    VkApplicationInfo appInfo = {};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = APPNAME;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion         = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    assert(vkCreateInstance(&createInfo, NULL, &instance) == VK_SUCCESS);
}
