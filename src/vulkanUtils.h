#ifndef VULKAN_UTILS_H
#define VULKAN_UTILS_H

#include <vulkan/vulkan.h>
#include <vector>

// Struct for holding all swapchain relevant data.
struct SwapchainData {
    VkSwapchainKHR swapchain;
    uint32_t imageCount;
    VkImage* swapchainImages;
    VkFormat swapchainFormat;
    VkExtent2D swapchainExtent;
};

struct SwapChainSupportDetails2 {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

SwapChainSupportDetails2 querySwapChainSupport2(VkPhysicalDevice device, VkSurfaceKHR surface);

SwapchainData createSwapchain(VkPhysicalDevice device, VkSurfaceKHR surface);

#endif
