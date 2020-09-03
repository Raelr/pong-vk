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
        VkPhysicalDevice physicalDevice             {VK_NULL_HANDLE};
        VkDebugUtilsMessengerEXT debugMessenger     {VK_NULL_HANDLE };
        VkInstance instance                         {nullptr};
        VkDevice logicalDevice                      {VK_NULL_HANDLE};
        VkSurfaceKHR surface                        {VK_NULL_HANDLE};
        QueueFamilyIndices indices                  {0};
        int framebufferWidth                        {0};
        int framebufferHeight                       {0};
        VkQueue graphicsQueue                       {VK_NULL_HANDLE};
        VkQueue presentQueue                        {VK_NULL_HANDLE};
    };

    Status checkValidationLayerSupport(uint32_t, VkLayerProperties*, const char**, uint32_t);
    Status checkVulkanExtensions(VulkanDeviceData*, bool);
    bool checkGlfwViability(const char** glfwExtensions, uint32_t glfwExtensionCount,
        VkExtensionProperties* vkExtensions, uint32_t vkExtensionCount);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device,VkSurfaceKHR surface);
    Status initialiseDebugUtilsMessenger(VkInstance, VkDebugUtilsMessengerEXT*);
    void populateDebugMessengerCI(VkDebugUtilsMessengerCreateInfoEXT&);
    void cleanupVulkanDevice(VulkanDeviceData*, bool);
}

#endif //PONG_VK_VULKANDEVICEDATA_H
