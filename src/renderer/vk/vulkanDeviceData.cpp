#include "vulkanDeviceData.h"
#include <GLFW/glfw3.h>
#include "validationLayers.h"
#include "swapchainData.h"

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

    Status initialiseDebugUtilsMessenger(VkInstance instance, VkDebugUtilsMessengerEXT* debugMessenger) {

        VkDebugUtilsMessengerCreateInfoEXT messengerInfo{};
        // Fill the struct with configuration data.
        populateDebugMessengerCI(messengerInfo);

        // Now we need to create the messenger and ensure it was successful:
        if (createDebugUtilsMessengerEXT(instance, &messengerInfo, nullptr, debugMessenger)
            != VK_SUCCESS) {
            // Throw an error and then stop execution if we weren't able to create the messenger
            return Status::INITIALIZATION_FAILURE;
        }

        return Status::SUCCESS;
    }

    void populateDebugMessengerCI(VkDebugUtilsMessengerCreateInfoEXT& messengerInfo) {
        populateDebugMessengerCreateInfo(messengerInfo);
    }

    void cleanupVulkanDevice(VulkanDeviceData* pDeviceData, bool enableValidationLayers) {

        free(pDeviceData->extensions);

        if (enableValidationLayers) {
            destroyDebugUtilsMessengerEXT(pDeviceData->instance, pDeviceData->debugMessenger, nullptr);
        }

        // Destroy window surface
        vkDestroySurfaceKHR(pDeviceData->instance, pDeviceData->surface, nullptr);

        // Destroy logical device
        vkDestroyDevice(pDeviceData->logicalDevice, nullptr);

        // Vulkan cleanup
        vkDestroyInstance(pDeviceData->instance, nullptr);
    }

    Status createGLFWWindowSurface(VkInstance instance, GLFWwindow* pWindow, VkSurfaceKHR* pSurface) {
        if (glfwCreateWindowSurface(instance, pWindow, nullptr, pSurface) != VK_SUCCESS) {
            ERROR("Failed to create window surface!");
            return Status::INITIALIZATION_FAILURE;
        }

        return Status::SUCCESS;
    }

    Status createPhysicalDevice(VulkanDeviceData* pDeviceData) {
        // Begin by querying which devices are supported by Vulkan on this machine.
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(pDeviceData->instance, &deviceCount, nullptr);

        if (deviceCount == 0) {
            ERROR("Failed to find GPUs that support Vulkan!");
            return Status::FAILURE;
        }

        // Now we query which specific devices we have
        VkPhysicalDevice devices[deviceCount];
        vkEnumeratePhysicalDevices(pDeviceData->instance, &deviceCount, devices);

        Status status = Status::FAILURE;

        // Now search for the GPU we want.
        for (size_t i = 0; i < deviceCount; i++) {

            // We need a device that has the queue families that we need.
            QueueFamilyIndices indices = findQueueFamilies(devices[i], pDeviceData->surface);

            // Now we query all available extensions on this device
            uint32_t extensionCount = 0;
            vkEnumerateDeviceExtensionProperties(devices[i], nullptr, &extensionCount, nullptr);

            VkExtensionProperties availableExtensions[extensionCount];
            vkEnumerateDeviceExtensionProperties(devices[i], nullptr, &extensionCount, availableExtensions);

            // Check if our device has all our required extensions
            size_t found = 0;
            for (size_t j = 0; j < pDeviceData->deviceExtensionCount; j++) {
                for (size_t k = 0; k < extensionCount; k++) {
                    if (strcmp(pDeviceData->deviceExtensions[j], availableExtensions[k].extensionName) == 0) {
                        found++;
                    }
                }
            }

            bool extensionsSupported = (found == pDeviceData->deviceExtensionCount);
            bool swapchainAdequate = false;

            if (extensionsSupported) {
                // Get the swap-chain details
                SwapchainSupportDetails supportDetails = querySwapchainSupport(devices[i], pDeviceData->surface);
                // Make sure that we have at least one supported format and one supported presentation mode.
                swapchainAdequate = supportDetails.formatCount > 0 && supportDetails.presentModesCount > 0;

                cleanupSwapchainSupportDetails(&supportDetails);
            }

            bool is_supported = (
                indices.graphicsFamily.has_value()
                && indices.presentFamily.has_value())
                // If all available extensions were 'ticked off' the set then we know we have all
                // required extensions.
                && extensionsSupported
                && swapchainAdequate;

            // If the device has all our required extensions and has a valid swap-chain and has our
            // required queue families, then we can use that device for rendering!
            if (is_supported) {
                pDeviceData->physicalDevice = devices[i];
                status = Status::SUCCESS;
                break;
            }
        }

        return status;
    }

    Status createLogicalDevice(VulkanDeviceData* pDeviceData) {
        // First, we need to query which queue families our physical device supports.
        pDeviceData->indices = findQueueFamilies(pDeviceData->physicalDevice, pDeviceData->surface);

        // It's possible that multiple queues are actually the same (such as graphics and present
        // queues). We therefore check if the two that we're looking for are the same.
        size_t createSize = (pDeviceData->indices.graphicsFamily.value()
                             == pDeviceData->indices.presentFamily.value()) ? 1 : 2;

        // Now we need an array for storing the queues that we want to create in future.
        VkDeviceQueueCreateInfo createInfos[createSize];
        uint32_t uniqueFamilies[createSize];

        uniqueFamilies[0] = pDeviceData->indices.graphicsFamily.value();

        // If we have multiple graphics families then add those into the array
        if (createSize == 2) {
            uniqueFamilies[1] = pDeviceData->indices.presentFamily.value();
        }

        // Each queue is given a priority - we'll set these ones to their maximum value
        float priority = 1.0f;

        // Now create a queue creation struct for each unique family that we have
        for (size_t i = 0; i < createSize; i++) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = uniqueFamilies[i];
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &priority;
            createInfos[i] = queueCreateInfo;
        }

        // Leave this empty for now - can add things later when we need.
        VkPhysicalDeviceFeatures deviceFeatures{};

        // Now we need to actually configure the logical device (note that it uses the queue info
        // and the device features we defined earlier).
        VkDeviceCreateInfo logicalDeviceInfo{};
        logicalDeviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        logicalDeviceInfo.pQueueCreateInfos = createInfos;
        logicalDeviceInfo.queueCreateInfoCount = static_cast<uint32_t>(createSize);
        logicalDeviceInfo.pEnabledFeatures = &deviceFeatures;
        logicalDeviceInfo.enabledExtensionCount = pDeviceData->deviceExtensionCount;
        logicalDeviceInfo.ppEnabledExtensionNames = pDeviceData->deviceExtensions;

        // Now we create the logical device using the data we've accumulated thus far.
        if (vkCreateDevice(pDeviceData->physicalDevice, &logicalDeviceInfo,nullptr,
                           &pDeviceData->logicalDevice) != VK_SUCCESS) {
            ERROR("Failed to create logical device!");
            return Status::INITIALIZATION_FAILURE;
        }

        // Now we just need to create the queue which will be used for our commands.

        // Create the queue using the struct and logical device we created earlier.
        vkGetDeviceQueue(pDeviceData->logicalDevice, pDeviceData->indices.graphicsFamily.value(), 0,
                         &pDeviceData->graphicsQueue);
        // Create the presentation queue using the struct.
        vkGetDeviceQueue(pDeviceData->logicalDevice,
                         pDeviceData->indices.presentFamily.value(), 0,
                         &pDeviceData->presentQueue);

        return Status::SUCCESS;
    }
}