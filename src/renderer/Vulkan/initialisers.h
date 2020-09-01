#ifndef PONG_VK_INITIALISERS_H
#define PONG_VK_INITIALISERS_H

#include <vulkan/vulkan.h>
#include "vulkanDeviceData.h"
#include "swapchainData.h"

namespace Renderer {

    VkApplicationInfo initialiseVulkanApplicationInfo(const char*, const char*, uint32_t, uint32_t, uint32_t);
    VkResult createSwapchain(SwapchainData*, VulkanDeviceData*);
    VkResult createImageViews(VkDevice, SwapchainData*);
}


#endif //PONG_VK_INITIALISERS_H
