#ifndef PONG_VK_SWAPCHAINDATA_H
#define PONG_VK_SWAPCHAINDATA_H

#include <vulkan/vulkan.h>
#include "../core.h"

namespace Renderer {

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
        VkSurfaceFormatKHR* formats;
        uint32_t formatCount;
        VkPresentModeKHR* presentModes;
        uint32_t presentModesCount;
    };

    SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device,VkSurfaceKHR surface);

};

#endif //PONG_VK_SWAPCHAINDATA_H
