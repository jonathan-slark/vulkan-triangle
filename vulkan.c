/* Based on:
 * https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/00_Base_code.html
 * Drawing a triangle in Vulkan, rewritten for C99 and Win32.
 * TODO:
 * Use terminate() on errors, instead of assert().
 * Change struct inilialisation method.
 * More comments?
 * Better error checks.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#include <windows.h>

#include "config.h"
#include "util.h"
#include "vulkan.h"
#include "win32.h"

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
    VkFramebuffer *framebuffers;
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
static void DestroyDebugUtilsMessengerEXT(
	VkInstance instance,
	VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks* pAllocator
	);
static void populatedebugci(VkDebugUtilsMessengerCreateInfoEXT *ci);
static void createdebugmessenger(void);
static void destroydebugmessenger(void);
#endif // DEBUG
static void createinstance(void);
static void destroyinstance(void);
static QueueFamilies findqueuefamilies(VkPhysicalDevice pd);
static uint32_t checkdeviceext(VkPhysicalDevice pd);
static uint32_t isdevicesuitable(VkPhysicalDevice pd);
static void pickphysicaldevice(void);
static void createlogicaldevice(void);
static void destroylogicaldevice(void);
static void createsurface(void);
static void destroysurface(void);
static void queryswapchainsupport(VkPhysicalDevice pd, VkSurfaceKHR surface);
static void freeswapchaindetails(void);
static VkSurfaceFormatKHR chooseswapspaceformat(void);
static VkPresentModeKHR chooseswappresentmode(void);
static VkExtent2D chooseswapextent(void);
static void createswapchain(void);
static void destroyswapchain(void);
static void createimageviews(void);
static void destroyimageviews(void);
static void creategraphicspipeline(void);
static char *createshadercode(const char *filename, size_t *size);
static void deleteshadercode(char **code);
static VkShaderModule createshadermodule(char *code, size_t size);
static void createrenderpass(void);
static void destroyrenderpass(void);
static void creategraphicspipeline(void);
static void destroygraphicspipeline(void);
static void createframebuffers(void);
static void destroyframebuffers(void);
static void createcommandpool(void);
static void destroycommandpool(void);
static void createcommandpool(void);
static void createcommandbuffer(void);
static void recordcommandbuffer(VkCommandBuffer commandbuffer,
	uint32_t imageindex);
static void createsyncobjects(void);
static void destroysyncobjects(void);
static void devicewait(void);

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
static const char *vertexshader   = "shaders/vertex.spv";
static const char *fragmentshader = "shaders/fragment.spv";
static const char *readonlybinary = "rb";
static VkPipelineLayout pipelinelayout;
static VkRenderPass renderpass;
static VkPipeline graphicspipeline;
static VkCommandPool commandpool;
static VkCommandBuffer commandbuffer;
static const uint32_t vertexcount = 3;
static VkSemaphore imageavailable;
static VkSemaphore renderfinished;
static VkFence inflight;

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
    for (i = 0; i < COUNT(layers); i++) {
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
    if (func == NULL)
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    else
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
}

void
DestroyDebugUtilsMessengerEXT(
	VkInstance instance,
	VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks* pAllocator
	)
{
    PFN_vkDestroyDebugUtilsMessengerEXT func =
	(PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance,
		"vkDestroyDebugUtilsMessengerEXT");
    if (func != NULL)
        func(instance, debugMessenger, pAllocator);
}

void
populatedebugci(VkDebugUtilsMessengerCreateInfoEXT *ci) {
    ci->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    ci->pNext = NULL;
    ci->flags = 0;
    ci->messageSeverity =
	/*VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
	VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |*/
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    ci->messageType =
	VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    ci->pfnUserCallback = debugCallback;
    ci->pUserData = NULL;
}

void
createdebugmessenger(void)
{
    VkDebugUtilsMessengerCreateInfoEXT ci;

    populatedebugci(&ci);
    if (CreateDebugUtilsMessengerEXT(instance, &ci, NULL, &debugmessenger) !=
	    VK_SUCCESS)
	terminate("Failed to set up debug messenger.");
}

void
destroydebugmessenger(void)
{
    DestroyDebugUtilsMessengerEXT(instance, debugmessenger, NULL);
}

#endif // DEBUG

void
vk_initialise(void)
{
    createinstance();
#ifdef DEBUG
    createdebugmessenger();
#endif /* DEBUG */
    createsurface();
    pickphysicaldevice();
    createlogicaldevice();
    createswapchain();
    createimageviews();
    createrenderpass();
    creategraphicspipeline();
    createframebuffers();
    createcommandpool();
    createcommandbuffer();
    createsyncobjects();
}

void
vk_terminate(void)
{
    devicewait();
    destroysyncobjects();
    destroycommandpool();
    destroyframebuffers();
    destroygraphicspipeline();
    destroyrenderpass();
    destroyimageviews();
    destroyswapchain();
    destroylogicaldevice();
    destroysurface();
#ifdef DEBUG
    destroydebugmessenger();
#endif /* DEBUG */
    destroyinstance();
}

void
createinstance(void)
{
    VkApplicationInfo ai = {
	.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
	.pNext = NULL,
	.pApplicationName = appname,
	.applicationVersion = appver,
	.pEngineName = NULL,
	.engineVersion = 0,
	.apiVersion = VK_API_VERSION_1_0
    };
#ifdef DEBUG
    VkDebugUtilsMessengerCreateInfoEXT debugci;
#endif /* DEBUG */
    VkInstanceCreateInfo ci = {
	.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
#ifdef DEBUG
	/* Main debug messenger doesn't exist in instance creation and
	 * destruction */
	.pNext = (VkDebugUtilsMessengerCreateInfoEXT *) &debugci,
#else
	.pNext = NULL,
#endif /* DEBUG */
	.flags = 0,
	.pApplicationInfo = &ai,
#ifdef DEBUG
	.enabledLayerCount = COUNT(layers),
	.ppEnabledLayerNames = layers,
#else
	.enabledLayerCount = 0,
	.ppEnabledLayerNames = NULL.
#endif /* DEBUG */
	.enabledExtensionCount = COUNT(exts),
	.ppEnabledExtensionNames = exts
    };

#ifdef DEBUG
    if (!checklayersupport())
	terminate("Validation layers requested, but not available.");

    populatedebugci(&debugci);
#endif /* DEBUG */
    if(vkCreateInstance(&ci, NULL, &instance) != VK_SUCCESS)
	terminate("Failed to create instance.\n");
}

void
destroyinstance(void)
{
    vkDestroyInstance(instance, NULL);
}

QueueFamilies
findqueuefamilies(VkPhysicalDevice pd)
{
    const uint32_t queuecount = 2;
    uint32_t qfpcount, i;
    QueueFamilies qf = { 0 };
    VkQueueFamilyProperties *qfps;
    VkBool32 present;

    /* Get available queue families */
    vkGetPhysicalDeviceQueueFamilyProperties(pd, &qfpcount, NULL);
    qfps = (VkQueueFamilyProperties *) malloc(qfpcount *
	    sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(pd, &qfpcount, qfps);

    /* Check for required queue families */
    for (i = 0; i < qfpcount; i++) {
	/* Supports graphics commands */
	if (qfps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
	    qf.graphics = i;
	    qf.count++;
	}

	/* Supports presenting to the surface */
	vkGetPhysicalDeviceSurfaceSupportKHR(pd, i, surface, &present);
	if (present) {
	    qf.present = i;
	    qf.count++;
	}

	if (qf.count == queuecount) {
	    qf.isSuitable = 1;
	    break;
	}
    }

    free(qfps);
    return qf;
}

uint32_t
checkdeviceext(VkPhysicalDevice pd)
{
    uint32_t availablecount, i, j, found;
    VkExtensionProperties *availableexts;

    /* Get available extensions */
    vkEnumerateDeviceExtensionProperties(pd, NULL, &availablecount, NULL);
    availableexts = (VkExtensionProperties *) malloc(availablecount *
	    sizeof(VkExtensionProperties));
    vkEnumerateDeviceExtensionProperties(pd, NULL, &availablecount,
	    availableexts);

    /* Check we have required extensions */
    for (i = 0; i < COUNT(deviceexts); i++) {
	found = 0;

	for (j = 0; j < availablecount; j++) {
	    if (strcmp(deviceexts[i], availableexts[j].extensionName) == 0) {
		found = 1;
		break;
	    }
	}

	if (!found) {
	    free(availableexts);
	    return 0;
	}
    }

    free(availableexts);
    return 1;
}

uint32_t
isdevicesuitable(VkPhysicalDevice pd)
{
    QueueFamilies qf = findqueuefamilies(pd);
    uint32_t extssupport = checkdeviceext(pd);
    uint32_t swapchainadequate = 0;

    if (extssupport) {
	queryswapchainsupport(pd, surface);
	swapchainadequate =
	    details.formats      != NULL &&
	    details.presentmodes != NULL;
    }

    return qf.isSuitable && extssupport && swapchainadequate;
}

void
pickphysicaldevice(void)
{
    uint32_t pdcount, i;
    VkPhysicalDevice *pds;

    /* Get available physical devices */
    vkEnumeratePhysicalDevices(instance, &pdcount, NULL);
    if (pdcount == 0)
	terminate("Failed to find GPUs with Vulkan support.");
    pds = (VkPhysicalDevice *) malloc(pdcount * sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(instance, &pdcount, pds);

    /* Select the first suitable device */
    for (i = 0; i < pdcount; i++)
	if (isdevicesuitable(pds[i])) {
	    physicaldevice = pds[i];
	    break;
	}

    free(pds);
    if (physicaldevice == VK_NULL_HANDLE)
	terminate("Failed to find a suitable GPU.");
}

void
createlogicaldevice(void)
{
    uint32_t i;
    QueueFamilies qf = findqueuefamilies(physicaldevice);
    const float prio = 1.0f;
    VkDeviceQueueCreateInfo *dqcis = (VkDeviceQueueCreateInfo *)
	malloc(qf.count * sizeof(VkDeviceQueueCreateInfo));
    VkPhysicalDeviceFeatures pdf = { 0 };
    VkDeviceCreateInfo dci = {
	.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
	.pNext = NULL,
	.flags = 0,
	.queueCreateInfoCount = qf.count,
	.pQueueCreateInfos = dqcis,
	/* Device layers don't exist now but keep code for older versions. */
#ifdef DEBUG
	.enabledLayerCount = COUNT(layers),
	.ppEnabledLayerNames = layers,
#else
	.enabledLayerCount = 0,
	.ppEnabledLayerNames = NULL,
#endif /* DEBUG */
	.enabledExtensionCount = COUNT(deviceexts),
	.ppEnabledExtensionNames = deviceexts,
	.pEnabledFeatures = &pdf
    };

    /* Create a queue for each queue family */
    for (i = 0; i < qf.count; i++) {
	dqcis[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	dqcis[i].pNext = NULL;
	dqcis[i].flags = 0;
	dqcis[i].queueFamilyIndex = i;
	dqcis[i].queueCount = 1;
	dqcis[i].pQueuePriorities = &prio;
    }

    /* Create the logical device */
    if (vkCreateDevice(physicaldevice, &dci, NULL, &device) != VK_SUCCESS)
	terminate("Failed to create logical device.");

    /* Get the queue handles */
    vkGetDeviceQueue(device, qf.graphics, 0, &graphics);
    vkGetDeviceQueue(device, qf.present,  0, &present);
}

void
destroylogicaldevice(void)
{
    vkDestroyDevice(device, NULL);
}

void
createsurface(void)
{
    VkWin32SurfaceCreateInfoKHR win32sci = {
	.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
	.pNext = NULL,
	.flags = 0,
	.hinstance = GetModuleHandle(NULL),
	.hwnd = hwnd
    };

    if (vkCreateWin32SurfaceKHR(instance, &win32sci, NULL, &surface) !=
	    VK_SUCCESS)
	terminate("Failed to create window surface.");
}

void
destroysurface(void)
{
    vkDestroySurfaceKHR(instance, surface, NULL);
}

void
queryswapchainsupport(VkPhysicalDevice pd, VkSurfaceKHR surface)
{
    /* Get basic surface capabilities */
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pd, surface,
	    &details.capabilities);

    /* Get supported surface formats */
    vkGetPhysicalDeviceSurfaceFormatsKHR(pd, surface,
	    &details.formatcount, NULL);
    if (details.formatcount > 0) {
	details.formats = (VkSurfaceFormatKHR *) malloc(details.formatcount *
		sizeof(VkSurfaceFormatKHR));
	vkGetPhysicalDeviceSurfaceFormatsKHR(pd, surface,
		&details.formatcount, details.formats);
    }

    /* Get supported presentation modes */
    vkGetPhysicalDeviceSurfacePresentModesKHR(pd, surface,
	    &details.presentmodecount, NULL);
    if (details.presentmodecount > 0) {
	details.presentmodes =
	    (VkPresentModeKHR *) malloc(details.presentmodecount *
		    sizeof(VkPresentModeKHR));
	vkGetPhysicalDeviceSurfacePresentModesKHR(pd, surface,
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

    /* Select format with correct colour channels and sRGB */
    for (i = 0; i < details.formatcount; i++)
	if (details.formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
		details.formats[i].colorSpace ==
		VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
	    return details.formats[i];

    /* Settle for first format */
    return details.formats[0];
}

VkPresentModeKHR
chooseswappresentmode(void)
{
    uint32_t i;

    /* Render frames as fast as possible without tearing */
    for (i = 0; i < details.presentmodecount; i++)
	if (details.presentmodes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
	    return details.presentmodes[i];

    /* Settle for wait for vertical sync */
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
	extent.width  = rcClient.right;
	extent.height = rcClient.bottom;

	if (extent.width < minwidth)
	    extent.width = minwidth;
	else if (extent.width > maxwidth)
	    extent.width = maxwidth;
	if (extent.height < minheight)
	    extent.height = minheight;
	else if (extent.height > maxheight)
	    extent.height = maxheight;

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
    const QueueFamilies qf = findqueuefamilies(physicaldevice);
    const uint32_t qfi[] = { qf.graphics, qf.present };
    VkSwapchainCreateInfoKHR ci = {
	.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
	.pNext = NULL,
	.surface = surface,
	.minImageCount = 0,
	.imageFormat = sf.format,
	.imageColorSpace = sf.colorSpace,
	.imageExtent = extent,
	.imageArrayLayers = 1,
	/* Render directly */
	.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
	/* Use exclusive for best perfomance, if queue familes are the same */
	.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
	.queueFamilyIndexCount = 0,
	.pQueueFamilyIndices = NULL,
	.preTransform = details.capabilities.currentTransform,
	/* Ignore alpha channel */
	.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
	.presentMode = pm,
	/* Ignore obscured pixels */
	.clipped = VK_TRUE,
	.oldSwapchain = VK_NULL_HANDLE
    };

    if (maximagecount > 0 && imagecount > maximagecount)
	imagecount = maximagecount;
    ci.minImageCount = imagecount;

    /* If queue families differ allow images to be used by different queues */
    if (qf.graphics != qf.present) {
	ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
	ci.queueFamilyIndexCount = 2;
	ci.pQueueFamilyIndices = qfi;
    }

    if (vkCreateSwapchainKHR(device, &ci, NULL, &swapchain.handle) !=
	    VK_SUCCESS)
	terminate("Failed to create swap chain.");
    freeswapchaindetails();

    /* Get the swap chain image handles */
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
    VkImageViewCreateInfo ci;

    swapchain.imageviews = (VkImageView *) malloc(swapchain.imagecount *
	    sizeof(VkImageView));

    for (i = 0; i < swapchain.imagecount; i++) {
	ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ci.pNext = NULL;
	ci.flags = 0;
	ci.image = swapchain.images[i];
	/* 2D textures */
	ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
	ci.format = swapchain.imageformat;
	/* Don't swizzle the color channels around */
	ci.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	ci.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	ci.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	ci.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	/* Colour targets with no mimmapping or multiple layers */
	ci.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
	ci.subresourceRange.baseMipLevel   = 0;
	ci.subresourceRange.levelCount     = 1;
	ci.subresourceRange.baseArrayLayer = 0;
	ci.subresourceRange.layerCount     = 1;

	if (vkCreateImageView(device, &ci, NULL, &swapchain.imageviews[i]) !=
		VK_SUCCESS)
	    terminate("Failed to create image views.");
    }
}

void
destroyimageviews(void)
{
    uint32_t i;

    for (i = 0; i < swapchain.imagecount; i++)
	vkDestroyImageView(device, swapchain.imageviews[i], NULL);

    free(swapchain.imageviews);
}

char *
createshadercode(const char *filename, size_t *size)
{
    FILE *fp;
    char *code;

    if ((fp = fopen(filename, readonlybinary)) == NULL)
	terminate("Could not open file %s\n", filename);

    if (fseek(fp, 0L, SEEK_END) != 0)
	terminate("Error on seeking file %s\n", filename);
    *size = ftell(fp);
    rewind(fp);
    code = (char *) malloc(*size * sizeof(char));
    if (fread(code, sizeof(char), *size, fp) < *size)
	terminate("Error reading file %s\n", filename);

    if (fclose(fp) == EOF)
	terminate("Error on closing file %s\n", filename);

    return code;
}

void
deleteshadercode(char **code)
{
    free(*code);
}

VkShaderModule
createshadermodule(char *code, size_t size)
{
    VkShaderModuleCreateInfo ci = { 0 };
    VkShaderModule sm;

    ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    ci.codeSize = size;
    ci.pCode = (const uint32_t *) code;

    if (vkCreateShaderModule(device, &ci, NULL, &sm) != VK_SUCCESS)
	terminate("Failed to create shader module");

    return sm;
}

void
createrenderpass(void)
{
    VkAttachmentDescription colorattachment = { 0 };
    VkAttachmentReference colorattachmentref = { 0 };
    VkSubpassDescription subpass = { 0 };
    VkRenderPassCreateInfo renderpassinfo = { 0 };
    VkSubpassDependency dependency = {
	.srcSubpass = VK_SUBPASS_EXTERNAL,
	.dstSubpass = 0,
	.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
	.srcAccessMask = 0,
	.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
	.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
	.dependencyFlags = 0
    };

    colorattachment.format = swapchain.imageformat;
    colorattachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorattachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorattachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorattachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorattachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorattachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorattachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    colorattachmentref.attachment = 0;
    colorattachmentref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorattachmentref;

    renderpassinfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderpassinfo.attachmentCount = 1;
    renderpassinfo.pAttachments = &colorattachment;
    renderpassinfo.subpassCount = 1;
    renderpassinfo.pSubpasses = &subpass;
    renderpassinfo.dependencyCount = 1;
    renderpassinfo.pDependencies = &dependency;

    if (vkCreateRenderPass(device, &renderpassinfo, NULL, &renderpass) != VK_SUCCESS)
	terminate("Failed to create render pass");
}

void
destroyrenderpass(void)
{
    vkDestroyRenderPass(device, renderpass, NULL);
}

void
creategraphicspipeline(void)
{
    char *vertexcode, *fragmentcode;
    size_t vertexcodesize, fragmentcodesize;
    VkShaderModule vertexsm, fragmentsm;
    VkPipelineShaderStageCreateInfo vertexpssci = { 0 }, fragmentpssci = { 0 };
    VkPipelineShaderStageCreateInfo shaderstagesci[2];
    VkDynamicState dynamicstates[] = {
	VK_DYNAMIC_STATE_VIEWPORT,
	VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo pdsci = { 0 };
    VkPipelineVertexInputStateCreateInfo pvisci = { 0 };
    VkPipelineInputAssemblyStateCreateInfo piasci = { 0 };
    VkPipelineViewportStateCreateInfo pvsci = { 0 };
    VkPipelineRasterizationStateCreateInfo prsci = { 0 };
    VkPipelineMultisampleStateCreateInfo pmsci = { 0 };
    VkPipelineColorBlendAttachmentState pcbas = { 0 };
    VkPipelineColorBlendStateCreateInfo pcbsci = { 0 };
    VkPipelineLayoutCreateInfo plci = { 0 };
    VkGraphicsPipelineCreateInfo gpci = { 0 };

    vertexcode   = createshadercode(vertexshader,   &vertexcodesize);
    fragmentcode = createshadercode(fragmentshader, &fragmentcodesize);
    vertexsm     = createshadermodule(vertexcode,   vertexcodesize);
    fragmentsm   = createshadermodule(fragmentcode, fragmentcodesize);

    vertexpssci.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexpssci.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexpssci.module = vertexsm;
    vertexpssci.pName = "main";

    fragmentpssci.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentpssci.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentpssci.module = fragmentsm;
    fragmentpssci.pName = "main";

    shaderstagesci[0] = vertexpssci;
    shaderstagesci[1] = fragmentpssci;

    pvisci.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    pvisci.vertexBindingDescriptionCount = 0;
    pvisci.vertexAttributeDescriptionCount = 0;

    piasci.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    piasci.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    piasci.primitiveRestartEnable = VK_FALSE;

    pvsci.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    pvsci.viewportCount = 1;
    pvsci.scissorCount = 1;

    prsci.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    prsci.depthClampEnable = VK_FALSE;
    prsci.rasterizerDiscardEnable = VK_FALSE;
    prsci.polygonMode = VK_POLYGON_MODE_FILL;
    prsci.lineWidth = 1.0f;
    prsci.cullMode = VK_CULL_MODE_BACK_BIT;
    prsci.frontFace = VK_FRONT_FACE_CLOCKWISE;
    prsci.depthBiasEnable = VK_FALSE;

    pmsci.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    pmsci.sampleShadingEnable = VK_FALSE;
    pmsci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    pcbas.colorWriteMask =
	VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
	VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    pcbas.blendEnable = VK_FALSE;

    pcbsci.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    pcbsci.logicOpEnable = VK_FALSE;
    pcbsci.logicOp = VK_LOGIC_OP_COPY;
    pcbsci.attachmentCount = 1;
    pcbsci.pAttachments = &pcbas;
    pcbsci.blendConstants[0] = 0.0f;
    pcbsci.blendConstants[1] = 0.0f;
    pcbsci.blendConstants[2] = 0.0f;
    pcbsci.blendConstants[3] = 0.0f;

    pdsci.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    pdsci.dynamicStateCount = COUNT(dynamicstates);
    pdsci.pDynamicStates = dynamicstates;

    plci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    plci.setLayoutCount = 0;
    plci.pushConstantRangeCount = 0;

    if (vkCreatePipelineLayout(device, &plci, NULL, &pipelinelayout) != VK_SUCCESS)
	terminate("Failed to create pipeline layout");

    gpci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    gpci.stageCount = 2;
    gpci.pStages = shaderstagesci;
    gpci.pVertexInputState = &pvisci;
    gpci.pInputAssemblyState = &piasci;
    gpci.pViewportState = &pvsci;
    gpci.pRasterizationState = &prsci;
    gpci.pMultisampleState = &pmsci;
    gpci.pColorBlendState = &pcbsci;
    gpci.pDynamicState = &pdsci;
    gpci.layout = pipelinelayout;
    gpci.renderPass = renderpass;
    gpci.subpass = 0;
    gpci.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &gpci, NULL,
		&graphicspipeline) != VK_SUCCESS)
	terminate("Failed to create graphics pipeline");

    vkDestroyShaderModule(device, vertexsm,   NULL);
    vkDestroyShaderModule(device, fragmentsm, NULL);
    deleteshadercode(&vertexcode);
    deleteshadercode(&fragmentcode);
}

void
destroygraphicspipeline(void)
{
    vkDestroyPipeline(device, graphicspipeline, NULL);
    vkDestroyPipelineLayout(device, pipelinelayout, NULL);
}

void
createframebuffers(void)
{
    uint32_t i;
    VkFramebufferCreateInfo fci = { 0 };

    swapchain.framebuffers = (VkFramebuffer *) malloc(swapchain.imagecount *
	    sizeof(VkFramebuffer));

    for (i = 0; i < swapchain.imagecount; i++) {
	fci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fci.renderPass = renderpass;
	fci.attachmentCount = 1;
	fci.pAttachments = &swapchain.imageviews[i];
	fci.width = swapchain.extent.width;
	fci.height = swapchain.extent.height;
	fci.layers = 1;

	if (vkCreateFramebuffer(device, &fci, NULL, &swapchain.framebuffers[i]) != VK_SUCCESS)
	    terminate("Failed to create framebuffer");
    }
}

void
destroyframebuffers(void)
{
    uint32_t i;

    for (i = 0; i < swapchain.imagecount; i++)
	vkDestroyFramebuffer(device, swapchain.framebuffers[i], NULL);

    free(swapchain.framebuffers);
}

void
createcommandpool(void)
{
    QueueFamilies qf = findqueuefamilies(physicaldevice);
    VkCommandPoolCreateInfo cpci = {
	.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
	.pNext = NULL,
	.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
	.queueFamilyIndex = qf.graphics
    };

    if (vkCreateCommandPool(device, &cpci, NULL, &commandpool) != VK_SUCCESS)
	terminate("Failed to create command pool");
}

void
destroycommandpool(void)
{
    vkDestroyCommandPool(device, commandpool, NULL);
}

void
createcommandbuffer(void)
{
    VkCommandBufferAllocateInfo cbai = {
	.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
	.pNext = NULL,
	.commandPool = commandpool,
	.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
	.commandBufferCount = 1
    };

    if (vkAllocateCommandBuffers(device, &cbai, &commandbuffer) != VK_SUCCESS)
	terminate("Failed to allocate command buffers");
}

void
recordcommandbuffer(VkCommandBuffer commandbuffer, uint32_t imageindex)
{
    VkCommandBufferBeginInfo cbbi = {
	.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	.pNext = NULL,
	.flags = 0,
	.pInheritanceInfo = NULL
    };
    VkClearValue clearcolour = {{{ 0.0f, 0.0f, 0.0f, 1.0f }}};
    VkRenderPassBeginInfo rpbi = {
	.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
	.pNext = NULL,
	.renderPass = renderpass,
	.framebuffer = swapchain.framebuffers[imageindex],
	.renderArea.offset = { 0, 0 },
	.renderArea.extent = swapchain.extent,
	.clearValueCount = 1,
	.pClearValues = &clearcolour
    };
    VkViewport viewport = {
	.x = 0.0f,
	.y = 0.0f,
	.width = (float) swapchain.extent.width,
	.height = (float) swapchain.extent.height,
	.minDepth = 0.0f,
	.maxDepth = 1.0f
    };
    VkRect2D scissor = {
	.offset = { 0, 0 },
	.extent = swapchain.extent
    };

    if (vkBeginCommandBuffer(commandbuffer, &cbbi) != VK_SUCCESS)
	terminate("Failed to begin recording command buffer");

    vkCmdBeginRenderPass(commandbuffer, &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
	    graphicspipeline);
    vkCmdSetViewport(commandbuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandbuffer, 0, 1, &scissor);
    vkCmdDraw(commandbuffer, vertexcount, 1, 0, 0);
    vkCmdEndRenderPass(commandbuffer);

    if (vkEndCommandBuffer(commandbuffer) != VK_SUCCESS)
	terminate("Failed to record command buffer");
}

void
vk_drawframe(void)
{
    uint32_t imageindex;
    VkSemaphore waitsemaphores[] = { imageavailable };
    VkPipelineStageFlags waitstages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSemaphore signalsemaphores[] = { renderfinished };
    VkSubmitInfo submitinfo = {
	.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
	.pNext = NULL,
	.waitSemaphoreCount = 1,
	.pWaitSemaphores = waitsemaphores,
	.pWaitDstStageMask = waitstages,
	.commandBufferCount = 1,
	.pCommandBuffers = &commandbuffer,
	.signalSemaphoreCount = 1,
	.pSignalSemaphores = signalsemaphores
    };
    VkSwapchainKHR swapchains[] = { swapchain.handle };
    VkPresentInfoKHR presentinfo = {
	.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
	.pNext = NULL,
	.waitSemaphoreCount = 1,
	.pWaitSemaphores = signalsemaphores,
	.swapchainCount = 1,
	.pSwapchains = swapchains,
	.pImageIndices = &imageindex,
	.pResults = NULL
    };

    /* Wait for the previous frame to finish rendering. The fence is created in
     * the signaled state so the first call won't block. */
    vkWaitForFences(device, 1, &inflight, VK_TRUE, UINT64_MAX);
    vkResetFences(device, 1, &inflight);

    vkAcquireNextImageKHR(device, swapchain.handle, UINT64_MAX, imageavailable,
	    VK_NULL_HANDLE, &imageindex);

    vkResetCommandBuffer(commandbuffer, 0);
    recordcommandbuffer(commandbuffer, imageindex);

    if (vkQueueSubmit(graphics, 1, &submitinfo, inflight) != VK_SUCCESS)
	terminate("Failed to submit draw command buffer");

    vkQueuePresentKHR(present, &presentinfo);
}

void
createsyncobjects(void)
{
    VkSemaphoreCreateInfo sci = {
	.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
	.pNext = NULL,
	.flags = 0
    };
    VkFenceCreateInfo fci = {
	.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
	.pNext = NULL,
	.flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    if (vkCreateSemaphore(device, &sci, NULL, &imageavailable) != VK_SUCCESS ||
	vkCreateSemaphore(device, &sci, NULL, &renderfinished) != VK_SUCCESS ||
	vkCreateFence(device, &fci, NULL, &inflight) != VK_SUCCESS)
	terminate("Failed to create semaphores");
}

void
destroysyncobjects(void)
{
    vkDestroySemaphore(device, imageavailable, NULL);
    vkDestroySemaphore(device, renderfinished, NULL);
    vkDestroyFence(device, inflight, NULL);
}

void
devicewait(void)
{
    vkDeviceWaitIdle(device);
}
