#include <assert.h>
#include <windows.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#include "config.h"
#include "vulkan_debug.h"
#include "vulkan_device.h"
#include "vulkan_imageview.h"
#include "vulkan_instance.h"
#include "vulkan_physicaldevice.h"
#include "vulkan_surface.h"
#include "vulkan_swapchain.h"

// Based on:
// https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/00_Base_code.html

#ifdef DEBUG
const char *validationLayers[] = { VALIDATIONLAYER };

#define NUMLAYERS (sizeof validationLayers / sizeof validationLayers[0])

const char **getValidationLayers() {
    return validationLayers;
}

uint32_t getNumLayers() {
    return NUMLAYERS;
}
#endif // DEBUG

const char *extensions[] = {
    VK_KHR_SURFACE_EXTENSION_NAME,
    VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#ifdef DEBUG
    ,VK_EXT_DEBUG_UTILS_EXTENSION_NAME
#endif // DEBUG
};

#define NUMEXT (sizeof extensions / sizeof extensions[0])

VkInstance instance;

VkInstance getInstance() {
    return instance;
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

    // main debug messenger doesn't exist in instance creation and destruction
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
    populateDebugMessengerCreateInfo(&debugCreateInfo);
    createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
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
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
}

void termVulkan() {
    destroyImageViews();
    destroySwapChain();
    destroyLogicalDevice();
#ifdef DEBUG
    destroyDebugMessenger();
#endif
    destroySurface();
    destroyInstance();
}
