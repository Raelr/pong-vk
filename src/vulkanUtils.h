#ifndef VULKAN_UTILS_H
#define VULKAN_UTILS_H

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>

namespace VulkanUtils {

    // Struct for holding all swapchain relevant data.
    struct SwapchainData {
        VkSwapchainKHR swapchain;
        uint32_t imageCount;
        VkImage* swapchainImages;
        VkFormat swapchainFormat;
        VkExtent2D swapchainExtent;
    };

    struct SwapchainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
    };

    SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device,
        VkSurfaceKHR surface);

    SwapchainData createSwapchain(VkPhysicalDevice device, VkSurfaceKHR surface);

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR
        surface);
}

#endif
