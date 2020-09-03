#include "swapchainData.h"

namespace Renderer {

    SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {

        // instantiate a struct to store swapchain details.
        SwapchainSupportDetails details;

        // Now follow a familiar pattern and query all the support details
        // from Vulkan...
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
                                                  &details.capabilities);

        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount,
                                             nullptr);

        // Using a vector for the utility functions - statically resize the
        // data within it to hold·the data we need.
        if (formatCount != 0) {
            VkSurfaceFormatKHR* formats = static_cast<VkSurfaceFormatKHR *>(malloc(
                    formatCount * sizeof(VkSurfaceFormatKHR)));
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount,
                                                 formats);
            details.formats = formats;
            details.formatCount = formatCount;
        }

        uint32_t presentModeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

        // Same as above ^
        if (presentModeCount != 0) {
            VkPresentModeKHR* presentModes = static_cast<VkPresentModeKHR *>(malloc(
                    presentModeCount * sizeof(VkPresentModeKHR)));
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, presentModes);
            details.presentModes = presentModes;
            details.presentModesCount = presentModeCount;
        }

        // Return the details we need
        return details;
    }

    void cleanupSwapchainSupportDetails(SwapchainSupportDetails* details) {
        free(details->formats);
        free(details->presentModes);
    }
}