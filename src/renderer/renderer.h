#ifndef PONG_VK_RENDERER_H
#define PONG_VK_RENDERER_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "vk/vulkanUtils.h"
#include "renderer2D.h"
#include "core.h"
#include "vk/initialisers.h"

namespace Renderer {

    struct Renderer {
        // Vulkan Device Data
        VulkanDeviceData deviceData                 {nullptr};
        // Swapchain
        SwapchainData swapchainData                 { VK_NULL_HANDLE };
        // Renderer2D
        Renderer2D::Renderer2DData renderer2DData   { VK_NULL_HANDLE };
        // Sync objects
        uint32_t maxFramesInFlight                  {2};
        VkSemaphore* imageAvailableSemaphores       {nullptr};
        VkSemaphore* renderFinishedSemaphores       {nullptr};
        VkFence* inFlightFences                     {nullptr};
        VkFence* imagesInFlight                     {nullptr};
        uint32_t currentFrame                       {0};
        uint32_t imageIndex                         {0};
        VkCommandBuffer* commandBuffers             {nullptr};
    };

    // Device creation functions
    Status initialiseRenderer(Renderer*, bool, GLFWwindow*);
    Status createSyncObjects(Renderer*, uint32_t = 2);
    Status drawFrame(Renderer*, bool*, GLFWwindow*);

    // Code to handle window minimisation
    Status onWindowMinimised(GLFWwindow*, int*, int*);

    // Cleanup code
    Status cleanupRenderer(Renderer*,  bool);

    // Functions for pre-loading the renderer with data prior to creation
    // Default data
    void loadDefaultValidationLayers(Renderer*);
    void loadDefaultDeviceExtensions(Renderer*);
    // Custom data
    [[maybe_unused]] Status loadCustomValidationLayers(Renderer*, const char**, uint32_t);
    [[maybe_unused]] Status loadCustomDeviceExtensions(Renderer*, const char**, uint32_t);

    // Drawing
    Status drawQuad(Renderer*, glm::vec3, glm::vec3, float, glm::vec3);

    VkResult recreateSwapchain(Renderer* pRenderer);
}

#endif //PONG_VK_RENDERER_H
