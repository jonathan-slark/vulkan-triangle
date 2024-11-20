/* Based on:
 * https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/00_Base_code.html
 * Rewritten for Ansi C and Win32.
 */

#include <assert.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#include <windows.h>

#include "config.h"
#include "win32.h"

/* Macros*/
#define NUMLAYERS (sizeof layers / sizeof layers[0])
#define NUMEXTS   (sizeof exts   / sizeof exts[0])

/* Function declarations */
#ifdef DEBUG
static uint32_t checkvalidationlayersupport(void);
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData
	);
static VkResult CreateDebugUtilsMessengerEXT(
	VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pDebugMessenger
	);
static VkResult DestroyDebugUtilsMessengerEXT(
	VkInstance instance,
	VkDebugUtilsMessengerEXT debugmessenger,
	const VkAllocationCallbacks* pAllocator
	);
static void populatedebugci(VkDebugUtilsMessengerCreateInfoEXT *ci);
static void createdebugmessenger(void);
static void destroydebugmessenger(void);
#endif // DEBUG
void vk_initialise(void);
void vk_terminate(void);
static void createinstance(void);
static void destroyinstance(void);

/* Variables */
#ifdef DEBUG
static const char *layers[] = { "VK_LAYER_KHRONOS_validation" };
static const char *exts[] = {
    VK_KHR_SURFACE_EXTENSION_NAME,
    VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME
};
VkDebugUtilsMessengerEXT debugmessenger;
#else
static const char *exts[] = {
    VK_KHR_SURFACE_EXTENSION_NAME,
    VK_KHR_WIN32_SURFACE_EXTENSION_NAME
};
#endif /* DEBUG */
static VkInstance instance;

/* Function implementations */

#ifdef DEBUG

bool
checkvalidationlayersupport(void)
{
    uint32_t availablenum, i, j, found;
    VkLayerProperties *availablelayers;

    /* Get available layers */
    vkEnumerateInstanceLayerProperties(&availablenum, NULL);
    availablelayers = (VkLayerProperties *) malloc(availablenum *
	    sizeof(VkLayerProperties));
    vkEnumerateInstanceLayerProperties(&availablenum, availablelayers);

    /* Check if layers we need are available */
    for (i = 0; i < NUMLAYERS; i++) {
	found = 0;

	for (j = 0; j < availablenum; j++) {
	    if (strcmp(layers[i], availablelayers[j].layerName) == 0) {
		found = 1;
		break;
	    }
	}

	if (found) {
	    continue;
	} else {
	    free(availablelayers);
	    return 0;
	}
    }

    free(availablelayers);
    return 1;
}

VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData
    )
{
    UNUSED(messageSeverity);
    UNUSED(messageType);
    UNUSED(pUserData);

    fprintf(stderr, "%s\n", pCallbackData->pMessage);

    return VK_FALSE;
}

VkResult
CreateDebugUtilsMessengerEXT(
	VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pDebugMessenger
	)
{
    PFN_vkCreateDebugUtilsMessengerEXT func =
	(PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance,
		"vkCreateDebugUtilsMessengerEXT");
    if (func == NULL) {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    } else {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
}

VkResult
DestroyDebugUtilsMessengerEXT(
	VkInstance instance,
	VkDebugUtilsMessengerEXT debugmessenger,
	const VkAllocationCallbacks* pAllocator
	)
{
    PFN_vkDestroyDebugUtilsMessengerEXT func =
	(PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance,
		"vkDestroyDebugUtilsMessengerEXT");
    if (func == NULL) {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    } else {
        func(instance, debugmessenger, pAllocator);
	return VK_SUCCESS;
    }
}

void
populatedebugci(VkDebugUtilsMessengerCreateInfoEXT *ci) {
    ci->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    ci->messageSeverity =
	//VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
	//VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    ci->messageType =
	VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    ci->pfnUserCallback = debugCallback;
}

void
createdebugmessenger(void)
{
    VkDebugUtilsMessengerCreateInfoEXT ci = { 0 };

    populatedebugci(&ci);
    assert(CreateDebugUtilsMessengerEXT(instance, &ci, NULL, &debugmessenger)
	    == VK_SUCCESS);
}

void
destroydebugmessenger(void)
{
    assert(DestroyDebugUtilsMessengerEXT(instance, debugmessenger, NULL)
	    == VK_SUCCESS);
}

#endif // DEBUG

void
vk_initialise(void)
{
#ifdef DEBUG
    assert(checkvalidationlayersupport());
#endif /* DEBUG */
    createinstance();
#ifdef DEBUG
    createdebugmessenger();
#endif /* DEBUG */
    createsurface();
    pickphysicaldevice();
    createlogicaldevice();
    createswapchain();
    createimageviews();
}

void
vk_terminate(void)
{
    destroyimageviews();
    destroyswapchain();
    destroylogicaldevice();
#ifdef DEBUG
    destroydebugmessenger();
#endif /* DEBUG */
    destroysurface();
    destroyinstance();
}

void
createinstance(void)
{
    VkApplicationInfo ai                       = { 0 };
    VkInstanceCreateInfo ci                    = { 0 };
    VkDebugUtilsMessengerCreateInfoEXT debugci = { 0 };

    ai.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    ai.pApplicationName   = appname;
    ai.applicationVersion = appver;
    ai.apiVersion         = VK_API_VERSION_1_0;

    ci.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    ci.pApplicationInfo = &ai;
#ifdef DEBUG
    ci.enabledLayerCount       = NUMLAYERS;
    ci.ppEnabledLayerNames     = layers;
    ci.enabledExtensionCount   = NUMEXTS;
    ci.ppEnabledExtensionNames = exts;

    /* Main debug messenger doesn't exist in instance creation and
     * destruction */
    populatedebugci(&debugci);
    ci.pNext = (VkDebugUtilsMessengerCreateInfoEXT *) &debugci;
#endif /* DEBUG */

    assert(vkCreateInstance(&ci, NULL, &instance) == VK_SUCCESS);
}

void
destroyinstance(void)
{
    vkDestroyInstance(instance, NULL);
}

#ifndef VULKAN_PHYSICALDEVICE_H
#define VULKAN_PHYSICALDEVICE_H

#include <stdbool.h>
#include <vulkan/vulkan.h>

struct QueueFamilyIndices {
    uint32_t graphicsFamily;
    uint32_t presentFamily;
    uint32_t numFamilies;
    bool isSuitable;
};

void pickphysicaldevice();
VkPhysicalDevice getPhysicalDevice();
const char **getDeviceExtensions();
uint32_t getNumDeviceExtensions();
struct QueueFamilyIndices findQueueFamilies(VkPhysicalDevice);

#endif /* VULKAN_PHYSICALDEVICE_H */
#include <assert.h>
#include <string.h>
#include <vulkan/vulkan.h>
#include "vulkan_instance.h"
#include "vulkan_physicaldevice.h"
#include "vulkan_surface.h"
#include "vulkan_swapchain.h"

const char *deviceExtensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

#define NUMDEVICEEXTENSIONS (sizeof deviceExtensions / sizeof deviceExtensions[0])

VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

VkPhysicalDevice getPhysicalDevice() {
    return physicalDevice;
}

const char **getDeviceExtensions() {
    return deviceExtensions;
}

uint32_t getNumDeviceExtensions() {
    return NUMDEVICEEXTENSIONS;
}

struct QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
    /* Get available queue families */
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);
    VkQueueFamilyProperties queueFamilies[queueFamilyCount]; /* VLA */
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
	    queueFamilies);

    /* Check for required queue families */
    struct QueueFamilyIndices indices;
    indices.isSuitable = false;
    indices.numFamilies = 0;
    for (uint32_t i = 0; i < queueFamilyCount; i++) {
	/* Supports graphics commands */
	if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
	    indices.graphicsFamily = i;
	    indices.numFamilies++;

	    /* Supports presenting to the surface */
	    VkBool32 presentSupport = false;
	    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, getSurface(),
		    &presentSupport);
	    if (presentSupport) {
		indices.presentFamily = i;
		indices.numFamilies++;
		indices.isSuitable = true;
		break;
	    }
	}
    }

    return indices;
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
    /* Get available extensions */
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, NULL);
    VkExtensionProperties availableExtensions[extensionCount]; /* VLA */
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount,
	    availableExtensions);

    /* Check we have required extensions */
    for (uint32_t i = 0; i < NUMDEVICEEXTENSIONS; i++) {
	bool extensionFound = false;

	for (uint32_t j = 0; j < extensionCount; j++) {
	    if (strcmp(deviceExtensions[i],
			availableExtensions[j].extensionName) == 0) {
		extensionFound = true;
		break;
	    }
	}

	if (extensionFound) {
	    continue;
	} else {
	    return false;
	}
    }

    return true;
}

bool isDeviceSuitable(VkPhysicalDevice device) {
    struct QueueFamilyIndices indices = findQueueFamilies(device);
    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
	querySwapChainSupport(device, getSurface());
	struct SwapChainSupportDetails details = getDetails();
	swapChainAdequate = details.formats != NULL &&
	    details.presentModes != NULL;
    }

    return indices.isSuitable && extensionsSupported && swapChainAdequate;
}

void pickphysicaldevice() {
    /* Get available physical devices */
    uint32_t deviceCount;
    VkInstance instance = instance;
    vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
    assert(deviceCount > 0);
    VkPhysicalDevice devices[deviceCount]; /* VLA */
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices);

    /* Select the first suitable device */
    for (uint32_t i = 0; i < deviceCount; i++) {
	if (isDeviceSuitable(devices[i])) {
	    physicalDevice = devices[i];
	    break;
	}
    }

    assert(physicalDevice != VK_NULL_HANDLE);
}
#ifndef VULKAN_DEVICE_H
#define VULKAN_DEVICE_H

#include <vulkan/vulkan.h>

VkDevice getDevice();
void createlogicaldevice();
void destroylogicaldevice();

#endif /* VULKAN_DEVICE_H */
#include <assert.h>
#include <vulkan/vulkan.h>
#include "util.h"
#include "vulkan_device.h"
#include "vulkan_instance.h"
#include "vulkan_physicaldevice.h"

VkDevice device;
VkQueue graphicsQueue;
VkQueue presentQueue;

VkDevice getDevice() {
    return device;
}

void createlogicaldevice() {
    /* Create a queue for each queue family */
    struct QueueFamilyIndices indices = findQueueFamilies(getPhysicalDevice());
    VkDeviceQueueCreateInfo queueCreateInfos[indices.numFamilies] = {}; /* VLA */
    float queuePriority = 1.0f;
    for (uint32_t i = 0; i < indices.numFamilies; i++) {
	queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfos[i].queueFamilyIndex = i;
	queueCreateInfos[i].queueCount = 1;
	queueCreateInfos[i].pQueuePriorities = &queuePriority;
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};
    
    /* Create the logical device */
    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos;
    createInfo.queueCreateInfoCount = indices.numFamilies;
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = getNumDeviceExtensions();
    createInfo.ppEnabledExtensionNames = getDeviceExtensions();
#ifdef DEBUG
    createInfo.enabledLayerCount = NUMLAYERS;
    createInfo.ppEnabledLayerNames = validationlayers;
#endif /* DEBUG */
    assert(vkCreateDevice(getPhysicalDevice(), &createInfo, NULL, &device) == VK_SUCCESS);

    /* Get the queues */
    vkGetDeviceQueue(device, indices.graphicsFamily, 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily, 0, &presentQueue);
}

void destroylogicaldevice() {
    vkDestroyDevice(device, NULL);
}
#ifndef VULKAN_SURFACE_H
#define VULKAN_SURFACE_H

#include <vulkan/vulkan.h>

VkSurfaceKHR getSurface();
void createsurface();
void destroysurface();

#endif /* VULKAN_SURFACE_H */
#include <assert.h>
#include <windows.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#include "vulkan_instance.h"
#include "vulkan_surface.h"
#include "win32.h"

VkSurfaceKHR surface;

VkSurfaceKHR getSurface() {
    return surface;
}

void createsurface() {
    VkWin32SurfaceCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = hwnd;
    createInfo.hinstance = GetModuleHandle(NULL);

    assert(vkCreateWin32SurfaceKHR(instance, &createInfo, NULL, &surface) == VK_SUCCESS);
}

void destroysurface() {
    vkDestroySurfaceKHR(instance, surface, NULL);
}
#ifndef VULKAN_SWAPCHAIN_H
#define VULKAN_SWAPCHAIN_H

#include <vulkan/vulkan.h>

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR *formats;
    VkPresentModeKHR *presentModes;
    uint32_t formatCount;
    uint32_t presentModeCount;
};

struct SwapChainSupportDetails getDetails();
VkImage *getSwapChainImages();
uint32_t getSwapChainImageNum();
VkFormat getSwapChainImageFormat();
void querySwapChainSupport(VkPhysicalDevice, VkSurfaceKHR);
void freeSwapChainSupport();
void createswapchain();
void destroyswapchain();

#endif /* VULKAN_SWAPCHAIN_H */
#include <assert.h>
#include <stdlib.h>
#include <windows.h>
#include <vulkan/vulkan.h>
#include "vulkan_device.h"
#include "vulkan_physicaldevice.h"
#include "vulkan_surface.h"
#include "vulkan_swapchain.h"
#include "win32.h"

struct SwapChainSupportDetails details;

VkSwapchainKHR swapChain;
VkImage *swapChainImages;
uint32_t swapChainImageNum;
VkFormat swapChainImageFormat;
VkExtent2D swapChainExtent;

struct SwapChainSupportDetails getDetails() {
    return details;
}

VkImage *getSwapChainImages() {
    return swapChainImages;
}

uint32_t getSwapChainImageNum() {
    return swapChainImageNum;
}

VkFormat getSwapChainImageFormat() {
    return swapChainImageFormat;
}

void querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
    /* Get basic surface capabilities */
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
	    &details.capabilities);

    /* Get supported surface formats */
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface,
	    &details.formatCount, NULL);
    if (details.formatCount > 0) {
	details.formats = (VkSurfaceFormatKHR *) malloc(details.formatCount *
		sizeof(VkSurfaceFormatKHR));
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface,
		&details.formatCount, details.formats);
    }

    /* Get supported presentation modes */
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
	    &details.presentModeCount, NULL);
    if (details.presentModeCount > 0) {
	details.presentModes =
	    (VkPresentModeKHR *) malloc(details.presentModeCount *
		    sizeof(VkPresentModeKHR));
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
		&details.presentModeCount, details.presentModes);
    }
}

void freeSwapChainSupport() {
    free(details.formats);
    free(details.presentModes);
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat() {
    for (uint32_t i = 0; i < details.formatCount; i++) {
	if (details.formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
		details.formats[i].colorSpace ==
		VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
	    return details.formats[i];
	}
    }

    return details.formats[0];
}

VkPresentModeKHR chooseSwapPresentMode() {
    for (uint32_t i = 0; i < details.presentModeCount; i++) {
        if (details.presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            return details.presentModes[i];
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapExtent() {
    VkExtent2D actualExtent;
    RECT rcClient;
    const uint32_t minWidth  = details.capabilities.minImageExtent.width;
    const uint32_t maxWidth  = details.capabilities.maxImageExtent.width;
    const uint32_t minHeight = details.capabilities.minImageExtent.height;
    const uint32_t maxHeight = details.capabilities.maxImageExtent.height;

    /* Check if width and height don't match the resolution */
    if (details.capabilities.currentExtent.width == UINT32_MAX) {
	GetClientRect(hwnd, &rcClient);
	actualExtent.width = rcClient.right;
	actualExtent.height = rcClient.bottom;

	if (actualExtent.width < minWidth) {
	    actualExtent.width = minWidth;
	} else if (actualExtent.width > maxWidth) {
	    actualExtent.width = maxWidth;
	}
	if (actualExtent.height < minHeight) {
	    actualExtent.height = minHeight;
	} else if (actualExtent.height > maxHeight) {
	    actualExtent.height = maxHeight;
	}

	return actualExtent;
    } else {
	return details.capabilities.currentExtent;
    }
}

void createswapchain() {
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat();
    VkPresentModeKHR presentMode = chooseSwapPresentMode();
    VkExtent2D extent = chooseSwapExtent();

    uint32_t imageCount = details.capabilities.minImageCount + 1;
    const uint32_t maxImageCount = details.capabilities.maxImageCount;
    if (maxImageCount > 0 && imageCount > maxImageCount) {
	imageCount = maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = getSurface();

    /* Swap chain images */
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    /* Use exclusive if the queue families are the same */
    struct QueueFamilyIndices indices = findQueueFamilies(getPhysicalDevice());
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily,
	indices.presentFamily };
    if (indices.graphicsFamily == indices.presentFamily) {
	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    } else {
	createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
	createInfo.queueFamilyIndexCount = 2;
	createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }

    createInfo.preTransform = details.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    assert(vkCreateSwapchainKHR(getDevice(), &createInfo, NULL, &swapChain) == VK_SUCCESS);
    freeSwapChainSupport();

    vkGetSwapchainImagesKHR(getDevice(), swapChain, &imageCount, NULL);
    swapChainImages = (VkImage *) malloc(imageCount * sizeof(VkImage));
    swapChainImageNum = imageCount;
    vkGetSwapchainImagesKHR(getDevice(), swapChain, &imageCount, swapChainImages);

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

void destroyswapchain() {
    vkDestroySwapchainKHR(getDevice(), swapChain, NULL);
    free(swapChainImages);
}
#ifndef VULKAN_IMAGEVIEW_H
#define VULKAN_IMAGEVIEW_H

void createimageviews();
void destroyimageviews();

#endif /* VULKAN_IMAGEVIEW_H */
#include <assert.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>
#include "vulkan_device.h"
#include "vulkan_imageview.h"
#include "vulkan_swapchain.h"

VkImageView *swapChainImageViews;

void createimageviews() {
    VkImage *swapChainImages = getSwapChainImages();
    uint32_t swapChainImageNum = getSwapChainImageNum();

    swapChainImageViews = (VkImageView *) malloc(swapChainImageNum *
	    sizeof(VkImageView));

    for (uint32_t i = 0; i < swapChainImageNum; i++) {
	VkImageViewCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.image = swapChainImages[i];
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = getSwapChainImageFormat();
	createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
	createInfo.subresourceRange.baseMipLevel   = 0;
	createInfo.subresourceRange.levelCount     = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount     = 1;

	assert(vkCreateImageView(getDevice(), &createInfo, NULL,
		    &swapChainImageViews[i]) == VK_SUCCESS);
    }
}

void destroyimageviews() {
    uint32_t swapChainImageNum = getSwapChainImageNum();

    for (uint32_t i = 0; i < swapChainImageNum; i++) {
	vkDestroyImageView(getDevice(), swapChainImageViews[i], NULL);
    }

    free(swapChainImageViews);
}
