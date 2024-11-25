/* Based on:
 * https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/00_Base_code.html
 * Drawing a triangle in Vulkan, rewritten for C99 and Win32.
 * TODO:
 * Use terminate() on errors, instead of assert().
 */

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#include <windows.h>

#include "config.h"
#include "util.h"
#include "vulkan.h"
#include "win32.h"

/* Macros */
#define LAYERSCOUNT        (sizeof layers        / sizeof layers[0])
#define EXTSCOUNT          (sizeof exts          / sizeof exts[0])
#define DEVICEEXTSCOUNT    (sizeof deviceexts    / sizeof deviceexts[0])
#define DYNAMICSTATESCOUNT (sizeof dynamicstates / sizeof dynamicstates[0])

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
static void terminate(const char *fmt, ...);
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
    if (func == NULL)
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    else
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
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
	/*VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
	VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |*/
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
terminate(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    exit(EXIT_FAILURE);
}

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
    createrenderpass();
    creategraphicspipeline();
}

void
vk_terminate(void)
{
    destroygraphicspipeline();
    destroyrenderpass();
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
findqueuefamilies(VkPhysicalDevice pd)
{
    uint32_t count, i;
    QueueFamilies qf = { 0 };
    VkQueueFamilyProperties *qfps;
    VkBool32 support;

    /* Get available queue families */
    vkGetPhysicalDeviceQueueFamilyProperties(pd, &count, NULL);
    qfps = (VkQueueFamilyProperties *) malloc(count *
	    sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(pd, &count, qfps);

    /* Check for required queue families */
    for (i = 0; i < count; i++) {
	/* Supports graphics commands */
	if (qfps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
	    qf.graphics = i;
	    qf.count++;

	    /* Supports presenting to the surface */
	    vkGetPhysicalDeviceSurfaceSupportKHR(pd, i, surface, &support);
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
    assert(pdcount > 0);
    pds = (VkPhysicalDevice *) malloc(pdcount * sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(instance, &pdcount, pds);

    /* Select the first suitable device */
    for (i = 0; i < pdcount; i++)
	if (isdevicesuitable(pds[i])) {
	    physicaldevice = pds[i];
	    break;
	}

    free(pds);
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

    for (i = 0; i < details.formatcount; i++)
	if (details.formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
		details.formats[i].colorSpace ==
		VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
	    return details.formats[i];

    return details.formats[0];
}

VkPresentModeKHR
chooseswappresentmode(void)
{
    uint32_t i;

    for (i = 0; i < details.presentmodecount; i++)
	if (details.presentmodes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
	    return details.presentmodes[i];

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
    VkSwapchainCreateInfoKHR ci = { 0 };
    const QueueFamilies qf = findqueuefamilies(physicaldevice);
    const uint32_t qfi[] = { qf.graphics, qf.present };

    if (maximagecount > 0 && imagecount > maximagecount)
	imagecount = maximagecount;

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
    pdsci.dynamicStateCount = DYNAMICSTATESCOUNT;
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
