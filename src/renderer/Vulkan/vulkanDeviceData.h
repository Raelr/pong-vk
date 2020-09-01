#ifndef PONG_VK_VULKANDEVICEDATA_H
#define PONG_VK_VULKANDEVICEDATA_H

#include <vulkan/vulkan.h>
#include <optional>
#include "../core.h"

namespace Renderer {

    // Struct storing all queue families
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
    };

    struct VulkanDeviceData {
        // Validation layers and extensions
        const char** validationLayers               {nullptr};
        uint32_t validationLayerCount               {0};
        const char** extensions                     {nullptr};
        uint32_t extensionCount                     {0};
        const char** deviceExtensions               {nullptr};
        uint32_t deviceExtensionCount               {0};
        // Device-specific data
        VkPhysicalDevice physicalDevice;
        VkDevice logicalDevice;
        VkSurfaceKHR surface;
        QueueFamilyIndices indices;
        int framebufferWidth;
        int framebufferHeight;
        VkQueue graphicsQueue;
        VkQueue presentQueue;
    };

    Status checkValidationLayerSupport(uint32_t, VkLayerProperties*, const char**, uint32_t);
    Status checkVulkanExtensions(VulkanDeviceData*, bool);
    bool checkGlfwViability(const char** glfwExtensions, uint32_t glfwExtensionCount,
        VkExtensionProperties* vkExtensions, uint32_t vkExtensionCount);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device,VkSurfaceKHR surface);
}



#endif //PONG_VK_VULKANDEVICEDATA_H
