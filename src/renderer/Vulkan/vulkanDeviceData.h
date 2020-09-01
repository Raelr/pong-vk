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
}



#endif //PONG_VK_VULKANDEVICEDATA_H
