/* Based on:
 * https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/00_Base_code.html
 * Drawing a triangle in Vulkan, rewritten for C99 and Win32.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#include <windows.h>

#include "config.h"
#include "util.h"
#include "win32.h"

/* Macros */
#define LAYERSCOUNT     (sizeof layers     / sizeof layers[0])
#define EXTSCOUNT       (sizeof exts       / sizeof exts[0])
#define DEVICEEXTSCOUNT (sizeof deviceexts / sizeof deviceexts[0])

/* Types */

typedef struct {
    uint32_t graphics;
    uint32_t present;
    uint32_t count;
    uint32_t isSuitable;
} QueueFamilies;

typedef struct {
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR *formats;
    VkPresentModeKHR *presentmodes;
    uint32_t formatcount;
    uint32_t presentmodecount;
} SwapChainDetails;

typedef struct {
    VkSwapchainKHR handle;
    VkImage *images;
    uint32_t imagecount;
    VkFormat imageformat;
    VkExtent2D extent;
    VkImageView *imageviews;
} SwapChain;

/* Function declarations */
#ifdef DEBUG
static uint32_t checklayersupport(void);
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
static QueueFamilies findqueuefamilies(VkPhysicalDevice device);
static uint32_t checkdeviceext(VkPhysicalDevice device);
static uint32_t isdevicesuitable(VkPhysicalDevice device);
static void pickphysicaldevice(void);
static void createlogicaldevice(void);
static void destroylogicaldevice(void);
static void createsurface(void);
static void destroysurface(void);
static void queryswapchainsupport(VkPhysicalDevice device, VkSurfaceKHR surface);
static void freeswapchaindetails(void);
static VkSurfaceFormatKHR chooseswapspaceformat(void);
static VkPresentModeKHR chooseswappresentmode(void);
static VkExtent2D chooseswapextent(void);
static void createswapchain(void);
static void destroyswapchain(void);
static void createimageviews(void);
static void destroyimageviews(void);

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
static const char *deviceexts[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
static VkInstance instance;
static VkPhysicalDevice physicaldevice = VK_NULL_HANDLE;
static SwapChainDetails details;
static VkDevice device;
static VkQueue graphics;
static VkQueue present;
static VkSurfaceKHR surface;
static SwapChain swapchain;

/* Function implementations */

#ifdef DEBUG

uint32_t
checklayersupport(void)
{
    uint32_t availablecount, i, j, found;
    VkLayerProperties *availablelayers;

    /* Get available layers */
    vkEnumerateInstanceLayerProperties(&availablecount, NULL);
    availablelayers = (VkLayerProperties *) malloc(availablecount *
	    sizeof(VkLayerProperties));
    vkEnumerateInstanceLayerProperties(&availablecount, availablelayers);

    /* Check if layers we need are available */
    for (i = 0; i < LAYERSCOUNT; i++) {
	found = 0;

	for (j = 0; j < availablecount; j++) {
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
    assert(checklayersupport());
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
    ci.enabledLayerCount       = LAYERSCOUNT;
    ci.ppEnabledLayerNames     = layers;
    ci.enabledExtensionCount   = EXTSCOUNT;
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

QueueFamilies
findqueuefamilies(VkPhysicalDevice device)
{
    uint32_t count, i;
    QueueFamilies qf = { 0 };
    VkQueueFamilyProperties *qfps;
    VkBool32 support;

    /* Get available queue families */
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, NULL);
    qfps = (VkQueueFamilyProperties *) malloc(count *
	    sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, qfps);

    /* Check for required queue families */
    for (i = 0; i < count; i++) {
	/* Supports graphics commands */
	if (qfps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
	    qf.graphics = i;
	    qf.count++;

	    /* Supports presenting to the surface */
	    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &support);
	    if (support) {
		qf.present = i;
		qf.count++;
		qf.isSuitable = 1;
		break;
	    }
	}
    }

    free(qfps);
    return qf;
}

uint32_t
checkdeviceext(VkPhysicalDevice device)
{
    uint32_t availablecount, i, j, found;
    VkExtensionProperties *availableexts;

    /* Get available extensions */
    vkEnumerateDeviceExtensionProperties(device, NULL, &availablecount, NULL);
    availableexts = (VkExtensionProperties *) malloc(availablecount *
	    sizeof(VkExtensionProperties));
    vkEnumerateDeviceExtensionProperties(device, NULL, &availablecount,
	    availableexts);

    /* Check we have required extensions */
    for (i = 0; i < DEVICEEXTSCOUNT; i++) {
	found = 0;

	for (j = 0; j < availablecount; j++) {
	    if (strcmp(deviceexts[i], availableexts[j].extensionName) == 0) {
		found = 1;
		break;
	    }
	}

	if (found) {
	    continue;
	} else {
	    free(availableexts);
	    return 0;
	}
    }

    free(availableexts);
    return 1;
}

uint32_t
isdevicesuitable(VkPhysicalDevice device)
{
    QueueFamilies qf = findqueuefamilies(physicaldevice);
    uint32_t extssupport = checkdeviceext(device);
    uint32_t swapchainadequate = 0;

    if (extssupport) {
	queryswapchainsupport(device, surface);
	swapchainadequate =
	    details.formats      != NULL &&
	    details.presentmodes != NULL;
    }

    return qf.isSuitable && extssupport && swapchainadequate;
}

void
pickphysicaldevice(void)
{
    uint32_t devicecount, i;
    VkPhysicalDevice *devices;

    /* Get available physical devices */
    vkEnumeratePhysicalDevices(instance, &devicecount, NULL);
    assert(devicecount > 0);
    devices = (VkPhysicalDevice *) malloc(devicecount *
	    sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(instance, &devicecount, devices);

    /* Select the first suitable device */
    for (i = 0; i < devicecount; i++) {
	if (isdevicesuitable(devices[i])) {
	    physicaldevice = devices[i];
	    break;
	}
    }

    free(devices);
    assert(physicaldevice != VK_NULL_HANDLE);
}

void
createlogicaldevice(void)
{
    uint32_t i;
    QueueFamilies qf = findqueuefamilies(physicaldevice);
    const float prio = 1.0f;
    VkDeviceQueueCreateInfo *cis = (VkDeviceQueueCreateInfo *)
	calloc(qf.count, sizeof(VkDeviceQueueCreateInfo));
    VkPhysicalDeviceFeatures df = { 0 };
    VkDeviceCreateInfo ci = { 0 };

    /* Create a queue for each queue family */
    for (i = 0; i < qf.count; i++) {
	cis[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	cis[i].queueFamilyIndex = i;
	cis[i].queueCount = 1;
	cis[i].pQueuePriorities = &prio;
    }

    /* Create the logical device */
    ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    ci.pQueueCreateInfos = cis;
    ci.queueCreateInfoCount = qf.count;
    ci.pEnabledFeatures = &df;
    ci.enabledExtensionCount = DEVICEEXTSCOUNT;
    ci.ppEnabledExtensionNames = deviceexts;
#ifdef DEBUG
    ci.enabledLayerCount = LAYERSCOUNT;
    ci.ppEnabledLayerNames = layers;
#endif /* DEBUG */
    assert(vkCreateDevice(physicaldevice, &ci, NULL, &device) == VK_SUCCESS);

    /* Get the queues */
    vkGetDeviceQueue(device, qf.graphics, 0, &graphics);
    vkGetDeviceQueue(device, qf.present, 0, &present);
}

void
destroylogicaldevice(void)
{
    vkDestroyDevice(device, NULL);
}

void
createsurface(void)
{
    VkWin32SurfaceCreateInfoKHR ci = { 0 };

    ci.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    ci.hwnd = hwnd;
    ci.hinstance = GetModuleHandle(NULL);

    assert(vkCreateWin32SurfaceKHR(instance, &ci, NULL, &surface) ==
	    VK_SUCCESS);
}

void
destroysurface(void)
{
    vkDestroySurfaceKHR(instance, surface, NULL);
}

void
queryswapchainsupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    /* Get basic surface capabilities */
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
	    &details.capabilities);

    /* Get supported surface formats */
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface,
	    &details.formatcount, NULL);
    if (details.formatcount > 0) {
	details.formats = (VkSurfaceFormatKHR *) malloc(details.formatcount *
		sizeof(VkSurfaceFormatKHR));
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface,
		&details.formatcount, details.formats);
    }

    /* Get supported presentation modes */
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
	    &details.presentmodecount, NULL);
    if (details.presentmodecount > 0) {
	details.presentmodes =
	    (VkPresentModeKHR *) malloc(details.presentmodecount *
		    sizeof(VkPresentModeKHR));
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
		&details.presentmodecount, details.presentmodes);
    }
}

void
freeswapchaindetails(void)
{
    free(details.formats);
    free(details.presentmodes);
}

VkSurfaceFormatKHR
chooseswapspaceformat(void)
{
    uint32_t i;

    for (i = 0; i < details.formatcount; i++) {
	if (details.formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
		details.formats[i].colorSpace ==
		VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
	    return details.formats[i];
	}
    }

    return details.formats[0];
}

VkPresentModeKHR
chooseswappresentmode(void)
{
    uint32_t i;

    for (i = 0; i < details.presentmodecount; i++) {
        if (details.presentmodes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            return details.presentmodes[i];
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D
chooseswapextent(void)
{
    VkExtent2D extent;
    RECT rcClient;
    const uint32_t minwidth  = details.capabilities.minImageExtent.width;
    const uint32_t maxwidth  = details.capabilities.maxImageExtent.width;
    const uint32_t minheight = details.capabilities.minImageExtent.height;
    const uint32_t maxheight = details.capabilities.maxImageExtent.height;

    /* Check if width and height don't match the resolution */
    if (details.capabilities.currentExtent.width == UINT32_MAX) {
	GetClientRect(hwnd, &rcClient);
	extent.width = rcClient.right;
	extent.height = rcClient.bottom;

	if (extent.width < minwidth) {
	    extent.width = minwidth;
	} else if (extent.width > maxwidth) {
	    extent.width = maxwidth;
	}
	if (extent.height < minheight) {
	    extent.height = minheight;
	} else if (extent.height > maxheight) {
	    extent.height = maxheight;
	}

	return extent;
    } else {
	return details.capabilities.currentExtent;
    }
}

void
createswapchain(void)
{
    uint32_t imagecount = details.capabilities.minImageCount + 1;
    const VkSurfaceFormatKHR sf = chooseswapspaceformat();
    const VkPresentModeKHR pm = chooseswappresentmode();
    const VkExtent2D extent = chooseswapextent();
    const uint32_t maximagecount = details.capabilities.maxImageCount;
    VkSwapchainCreateInfoKHR ci = { 0 };
    const QueueFamilies qf = findqueuefamilies(physicaldevice);
    const uint32_t qfi[] = { qf.graphics, qf.present };

    if (maximagecount > 0 && imagecount > maximagecount) {
	imagecount = maximagecount;
    }

    ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    ci.surface = surface;

    /* Swap chain images */
    ci.minImageCount = imagecount;
    ci.imageFormat = sf.format;
    ci.imageColorSpace = sf.colorSpace;
    ci.imageExtent = extent;
    ci.imageArrayLayers = 1;
    ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    /* Use exclusive if the queue families are the same */
    if (qf.graphics == qf.present) {
	ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    } else {
	ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
	ci.queueFamilyIndexCount = 2;
	ci.pQueueFamilyIndices = qfi;
    }

    ci.preTransform = details.capabilities.currentTransform;
    ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    ci.presentMode = pm;
    ci.clipped = VK_TRUE;
    ci.oldSwapchain = VK_NULL_HANDLE;

    assert(vkCreateSwapchainKHR(device, &ci, NULL, &swapchain.handle) == VK_SUCCESS);
    freeswapchaindetails();

    vkGetSwapchainImagesKHR(device, swapchain.handle, &imagecount, NULL);
    swapchain.images = (VkImage *) malloc(imagecount * sizeof(VkImage));
    swapchain.imagecount = imagecount;
    vkGetSwapchainImagesKHR(device, swapchain.handle, &imagecount,
	    swapchain.images);

    swapchain.imageformat = sf.format;
    swapchain.extent = extent;
}

void
destroyswapchain(void)
{
    vkDestroySwapchainKHR(device, swapchain.handle, NULL);
    free(swapchain.images);
}

void
createimageviews(void)
{
    uint32_t i;
    VkImageViewCreateInfo ci = { 0 };

    swapchain.imageviews = (VkImageView *) malloc(swapchain.imagecount *
	    sizeof(VkImageView));

    for (i = 0; i < swapchain.imagecount; i++) {
	ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ci.image = swapchain.images[i];
	ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
	ci.format = swapchain.imageformat;
	ci.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	ci.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	ci.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	ci.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	ci.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
	ci.subresourceRange.baseMipLevel   = 0;
	ci.subresourceRange.levelCount     = 1;
	ci.subresourceRange.baseArrayLayer = 0;
	ci.subresourceRange.layerCount     = 1;

	assert(vkCreateImageView(device, &ci, NULL,
		    &swapchain.imageviews[i]) == VK_SUCCESS);
    }
}

void
destroyimageviews(void)
{
    uint32_t i;

    for (i = 0; i < swapchain.imagecount; i++) {
	vkDestroyImageView(device, swapchain.imageviews[i], NULL);
    }

    free(swapchain.imageviews);
}
