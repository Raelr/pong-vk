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
        VkFormat swapchainFormat;
        VkExtent2D swapchainExtent;
        VkImageView* pImageViews;
        VkImage* pImages;
    };
    // Struct storing details relating to swapchain extensions and
    // support.
    struct SwapchainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };
    // Struct storing all queue families
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
    };

    SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device,
        VkSurfaceKHR surface);

    VkResult createSwapchain(SwapchainData* data,
            VkPhysicalDevice physicalDevice,
            VkDevice device,
            VkSurfaceKHR surface, const uint32_t windowHeight,
            const uint32_t windowWidth,
            QueueFamilyIndices indices);

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR
        surface);
    
    void destroySwapchainImageData(SwapchainData);

    VkResult createImageViews(VkDevice, SwapchainData*);
}

#endif
