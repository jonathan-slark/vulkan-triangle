#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <vulkan/vulkan.h>
#include "util.h"
#include "vulkan_debug.h"
#include "vulkan_instance.h"

VkDebugUtilsMessengerEXT debugMessenger;

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

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    UNUSED(messageSeverity);
    UNUSED(messageType);
    UNUSED(pUserData);

    fprintf(stderr, "%s\n", pCallbackData->pMessage);

    return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(
	VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {

    PFN_vkCreateDebugUtilsMessengerEXT func = 
	(PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func == NULL) {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    } else {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
}

VkResult DestroyDebugUtilsMessengerEXT(
	VkInstance instance,
	VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks* pAllocator) {

    PFN_vkDestroyDebugUtilsMessengerEXT func = 
	(PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func == NULL) {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    } else {
        func(instance, debugMessenger, pAllocator);
	return VK_SUCCESS;
    }
}

void initDebugMessenger() {
    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = NULL; // Optional

    assert(CreateDebugUtilsMessengerEXT(*getInstance(), &createInfo, NULL, &debugMessenger) == VK_SUCCESS);
}

void termDebugMessenger() {
    assert(DestroyDebugUtilsMessengerEXT(*getInstance(), debugMessenger, NULL) == VK_SUCCESS);
}
