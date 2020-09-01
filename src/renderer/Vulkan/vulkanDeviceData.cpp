#include "vulkanDeviceData.h"
#include <GLFW/glfw3.h>

namespace Renderer {

    Status checkValidationLayerSupport(
            uint32_t layerCount,
            VkLayerProperties* layerProperties,
            const char** pValidationLayers,
            uint32_t pValidationLayersCount
    ) {
        // Get the layer names themselves USING the count variable.
        vkEnumerateInstanceLayerProperties(&layerCount, layerProperties);
        uint8_t foundCount = 0;

        // Search to see whether the validation layers requested are supported by Vulkan.
        for (size_t i = 0; i < pValidationLayersCount; i++) {
            for (size_t j = 0; j < layerCount; j++) {
                if (strcmp(pValidationLayers[i], layerProperties[j].layerName) == 0) {
                    foundCount++;
                }
            }
        }

        // Throw an error and end the program if the validation layers aren't found.
        if (foundCount != pValidationLayersCount) {
            return Status::FAILURE;
        }

        INFO("Requested Validation layers exist!");

        return Status::SUCCESS;
    }

    Status checkVulkanExtensions(VulkanDeviceData* pDeviceData, bool enableValidationLayers) {

        uint32_t vulkanExtensionCount = 0;

        // This instance of the method will return the number of supported extensions.
        vkEnumerateInstanceExtensionProperties(nullptr, &vulkanExtensionCount, nullptr);

        VkExtensionProperties vkExtensions[vulkanExtensionCount];

        // This instance of the method will return the exact extensions supported
        vkEnumerateInstanceExtensionProperties(nullptr, &vulkanExtensionCount, vkExtensions);

        // Print out and display all extensions.
        INFO("Checking Extensions: ");

        for (size_t i = 0; i < vulkanExtensionCount; i++) {
            char* extension = vkExtensions[i].extensionName;
            INFO(extension);
        }

        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        if (!checkGlfwViability(glfwExtensions, glfwExtensionCount, vkExtensions, vulkanExtensionCount)) {
            return Status::INITIALIZATION_FAILURE;
        }

        uint32_t extensionCount = (enableValidationLayers) ? glfwExtensionCount += 1 : glfwExtensionCount;

        const char** extensions = static_cast<const char **>(malloc(extensionCount * sizeof(const char *)));

        if (enableValidationLayers) {
            extensions[extensionCount-1] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
        }

        for (size_t i = 0; i < extensionCount-1; i++) {
            extensions[i] = glfwExtensions[i];
            INFO(extensions[i]);
        }

        pDeviceData->extensions = extensions;
        pDeviceData->extensionCount = extensionCount;

        return Status::SUCCESS;
    }

    bool checkGlfwViability(const char** glfwExtensions, uint32_t glfwExtensionCount,
        VkExtensionProperties* vkExtensions, uint32_t vkExtensionCount) {

        bool success = false;
        size_t foundCount = 0;

        // Check if the GLFW extensions match the supported extensions by Vulkan.
        for (size_t i = 0; i < glfwExtensionCount; i++) {
            for (size_t j = 0; j < vkExtensionCount; j++) {
                if (strcmp(vkExtensions[j].extensionName, glfwExtensions[i]) == 0) {
                    foundCount++;
                }
            }
        }

        // Check if all GLFW extensions are supported by Vulkan.
        if (foundCount == glfwExtensionCount) {
            INFO("GLFW extensions are supported by Vulkan!");
            success = true;
        } else {
            ERROR("GLFW extensions are NOT supported by Vulkan!");
        }

        return success;
    }

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device,VkSurfaceKHR surface) {
        // A struct for storing the index of the queue family that the
        // device will be using
        QueueFamilyIndices indices;

        // Again, get the queue families that the device uses.
        uint32_t queueFamilyCount = 0;

        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,nullptr);

        VkQueueFamilyProperties* queueFamilies = new VkQueueFamilyProperties[queueFamilyCount];

        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

        // Now that we know the families, we can now assess the suitability
        // of this device.
        for (size_t i = 0; i < queueFamilyCount; i++) {
            // We search for a flag which specifies that the queue supports
            // graphics operations.
            // This is specified with the VK_QUEUE_GRAPHICS_BIT flag.
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface,&presentSupport);

            if (presentSupport) {
                indices.presentFamily = i;
            }
        }

        delete [] queueFamilies;

        return indices;
    }
}