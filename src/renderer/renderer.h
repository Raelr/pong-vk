#ifndef PONG_VK_RENDERER_H
#define PONG_VK_RENDERER_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "../vulkanUtils.h"
#include "renderer2D.h"

namespace Renderer {

    // A simple Enum for judging the success/failure of a function (kinda like VkResult)
    // TODO: Move this to a core header file
    enum Status {
        FAILURE = 0,
        INITIALIZATION_FAILURE = 1,
        SUCCESS = 2
    };

    struct Renderer {
        // Default parameters for our validation layer and extensions
        const char** validationLayers               {nullptr};
        uint32_t validationLayerCount               {0};
        const char** extensions                     {nullptr};
        uint32_t extensionCount                     {0};
        const char** deviceExtensions               {nullptr};
        uint32_t deviceExtensionCount               {0};
        // Vulkan structs:
        VkInstance instance                         {nullptr};
        VkDebugUtilsMessengerEXT debugMessenger     {nullptr};
        VulkanUtils::VulkanDeviceData deviceData    {nullptr};
        VulkanUtils::SwapchainData swapchainData    {nullptr};
        Renderer2D::Renderer2DData renderer2DData   {nullptr};
    };

    // Device creation functions
    Status initialiseRenderer(Renderer*, bool, GLFWwindow*);
    Status initialiseVulkanInstance(Renderer*, bool);
    Status checkVulkanExtensions(Renderer*, bool);
    Status checkValidationLayerSupport(uint32_t, VkLayerProperties*, const char**, uint32_t);
    bool checkGlfwViability(const char**, uint32_t, VkExtensionProperties*, uint32_t);
    Status initialiseDebugUtilsMessenger(VkInstance, VkDebugUtilsMessengerEXT*);
    Status createWindowSurface(VkInstance, GLFWwindow*, VkSurfaceKHR*);
    Status createPhysicalDevice(Renderer*);
    Status createLogicalDevice(Renderer*);

    // Cleanup code
    Status cleanupRenderer(Renderer*,  bool);

    // Functions for pre-loading the renderer with data prior to creation

    // Default data
    void loadDefaultValidationLayers(Renderer*);
    void loadDefaultDeviceExtensions(Renderer*);
    // Custom data
    [[maybe_unused]] Status loadCustomValidationLayers(Renderer*, const char**, uint32_t);
    [[maybe_unused]] Status loadCustomDeviceExtensions(Renderer*, const char**, uint32_t);
}

#endif //PONG_VK_RENDERER_H
