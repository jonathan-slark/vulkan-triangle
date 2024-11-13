#ifndef VULKAN_DEBUG_H
#define VULKAN_DEBUG_H

#include <stdbool.h>
#include <vulkan/vulkan.h>

#define VALIDATIONLAYER "VK_LAYER_KHRONOS_validation"

bool checkValidationLayerSupport();
void createDebugMessenger();
void destroyDebugMessenger();

#endif // VULKAN_DEBUG_H
