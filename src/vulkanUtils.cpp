#include "vulkanUtils.h"
//struct SwapchainData {
//    VkSwapchainKHR swapchain;
//    uint32_t imageCount;
//    VkImage* swapchainImages;
//    VkFormat swapchainFormat;
//    VkExtent2D swapchainExtent;
//};

//struct SwapChainSupportDetails2 {
//    VkSurfaceCapabilitiesKHR capabilities;
//    std::vector<VkSurfaceFormatKHR> formats;
//    std::vector<VkPresentModeKHR> presentModes;
//};

namespace VulkanUtils {

    SwapchainSupportDetails querySwapChainSupport(VkPhysicalDevice device, 
        VkSurfaceKHR surface){

        // instantiate a struct to store swapchain details.
        SwapchainSupportDetails details;

        // Now follow a familiar pattern and query all the support details from Vulkan...
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());

        // Using a vector for the utility functions - statically resize the data within it to hold·
        // the data we need.
        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }
       
        uint32_t presentModeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

        // Same as above ^
        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount,
                details.presentModes.data());
        }
       
        // Return the details we need
        return details;

    }

    SwapchainData createSwapchain(VkPhysicalDevice device, VkSurfaceKHR surface) {
        
        SwapchainData data{};
        
        // Do stuff with the swapchain

        return data;

    }

}
