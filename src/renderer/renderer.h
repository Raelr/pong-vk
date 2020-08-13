#ifndef PONG_VK_RENDERER_H
#define PONG_VK_RENDERER_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "../vulkanUtils.h"

namespace Renderer {

    enum Status {
        FAILURE = 0,
        INITIALIZATION_FAILURE = 1,
        SUCCESS = 2
    };

    struct Renderer {
        VkInstance instance;
        const char** validationLayers;
        uint32_t validationLayerCount;
        const char** extensions;
        uint32_t extensionCount;
        VkDebugUtilsMessengerEXT debugMessenger;
        VulkanUtils::VulkanDeviceData deviceData;
    };

    struct QuadData {


    };

    Status initialiseRenderer(Renderer*, bool, GLFWwindow*, const char** = nullptr, uint32_t = 0);
    Status initialiseVulkanInstance(Renderer*, bool);
    Status checkVulkanExtensions(Renderer*, bool);
    Status checkValidationLayerSupport(uint32_t, VkLayerProperties*, const char**, uint32_t);
    bool checkGlfwViability(const char**, uint32_t, VkExtensionProperties*, uint32_t);
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT&);
    Status initialiseDebugUtilsMessenger(VkInstance, VkDebugUtilsMessengerEXT*);
    Status createWindowSurface(VkInstance, GLFWwindow*, VkSurfaceKHR*);
    Status createPhysicalDevice(Renderer*);
    // Functions for pre-loading the renderer with data prior to creation
    void loadDefaultValidationLayers(Renderer* renderer);
    Status loadCustomValidationLayers(Renderer*, const char**, uint32_t);
}





#endif //PONG_VK_RENDERER_H
