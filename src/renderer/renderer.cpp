#include "renderer.h"
#include "../logger.h"
#include <cstring>
#include <GLFW/glfw3.h>

namespace Renderer {

    // A debug function callback - where the message data actually goes when triggered by the
    // validation layer.
    // This function uses three mactros to define it's signature (used to help Vulkan know that
    // this method is valid).
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,     // The severity of the message
            VkDebugUtilsMessageTypeFlagsEXT messageType,                // The actual TYPE of message
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,  // Struct with data of the message
            void* pUserData) {                                          // User defined data (optional)

        // In this case we simply print out the message itself to the console.
        ERROR(pCallbackData->pMessage);

        // Return value determines whether the call that triggers this message should be aborted.
        // Generally these should return false.
        return VK_FALSE;
    }

    // A proxy function which calls the vkCreateDebugUtilsMessengerEXT function.
    // The reason we need this function is because 'vkCreateDebugUtilsMessengerEXT' is an
    // extension function - meaning that it isn't automatically loaded into memory. As such,
    // we need to manually look up it's memory and call it from there. Vulkan provides us
    // with a utility function: 'vkGetInstanceProcAddr' for this exact purpose.
    static VkResult createDebugUtilsMessengerEXT(VkInstance instance,
                                                 const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                                 VkDebugUtilsMessengerEXT* pDebugMessenger) {

        // Store the function in a variable 'func'.
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
                instance, "vkCreateDebugUtilsMessengerEXT");

        // Make sure we got the correct function.
        if (func != nullptr) {
            // Call the function
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        } else {
            // Return an error
            return VkResult::VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    Status initialiseRenderer(
        Renderer* renderer,
        bool enableValidationLayers,
        GLFWwindow* window,
        const char** deviceExtensions,
        uint32_t deviceExtensionsCount// optional
    ) {

        // ========================== VALIDATION LAYER CHECKING ==============================

        uint32_t layerCount = 0;

        // Get the number of layers available for vulkan.
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        VkLayerProperties layerProperties[layerCount];

        if (enableValidationLayers) {

            if (checkValidationLayerSupport(layerCount,layerProperties, renderer->validationLayers,
                renderer->validationLayerCount) != Status::SUCCESS) {
                return Status::FAILURE;
            }
        }

        // ========================= VULKAN INSTANCE CREATION ================================

        renderer->debugMessenger = VK_NULL_HANDLE;

        if (checkVulkanExtensions(renderer, enableValidationLayers) == Status::FAILURE) {
            ERROR("No GLFW extensions available!");
            return Status::FAILURE;
        }

        if (initialiseVulkanInstance(renderer, enableValidationLayers) == Status::FAILURE) {
            return Status::INITIALIZATION_FAILURE;
        }

        if (enableValidationLayers) {
            if (initialiseDebugUtilsMessenger(renderer->instance, &renderer->debugMessenger)
                != Status::SUCCESS) {
                ERROR("Failed to create Debug utils messenger!");
                return Status::INITIALIZATION_FAILURE;
            }
        }

        // ============================ SURFACE CREATION ====================================

        INFO("Initialised Vulkan instance.");

        if (createWindowSurface(renderer->instance, window, &renderer->deviceData.surface)
            != Status::SUCCESS) {
            return Status::INITIALIZATION_FAILURE;
        }

        INFO("Retrieved Surface from GLFW.");

        // ========================= PHYSICAL DEVICE CREATION ===============================

        if (createPhysicalDevice(renderer) != Status::SUCCESS) {
            ERROR("Failed to create physical device!");
            return Status::INITIALIZATION_FAILURE;
        }

        INFO("Created physical device!");

        // ========================== LOGICAL DEVICE CREATION ===============================



        return Status::SUCCESS;
    }

    Status checkVulkanExtensions(Renderer* renderer, bool enableValidationLayers) {

        uint32_t vulkanExtensionCount = 0;

        // This instance of the method will return the number of supported extensions.
        vkEnumerateInstanceExtensionProperties(nullptr, &vulkanExtensionCount, nullptr);

        VkExtensionProperties vkExtensions[vulkanExtensionCount];

        // This instance of the method will return the exact extensions supported
        vkEnumerateInstanceExtensionProperties(nullptr, &vulkanExtensionCount, vkExtensions);

        // Print out and display all extensions.
        INFO("Checking Extensions: ");

        for (size_t i = 0; i < vulkanExtensionCount; i++) {
            char* extension = vkExtensions->extensionName;
            extension = strcat("\t", extension);
            INFO(extension);
        }

        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        if (!checkGlfwViability(glfwExtensions, glfwExtensionCount, vkExtensions, vulkanExtensionCount)) {
            return Status::INITIALIZATION_FAILURE;
        }

        uint32_t extensionCount = (enableValidationLayers) ? glfwExtensionCount += 1 : glfwExtensionCount;

        const char* extensions[extensionCount];

        if (enableValidationLayers) {
            extensions[extensionCount-1] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
        }

        for (size_t i = 0; i < extensionCount-1; i++) {
            extensions[i] = glfwExtensions[i];
        }

        renderer->extensions = extensions;
        renderer->extensionCount = extensionCount;

        return Status::SUCCESS;
    }

    Status initialiseVulkanInstance(Renderer* renderer, bool enableValidationLayers) {

        // ============================== INSTANCE CREATION =====================================

        // Define the configuration details of the vulkan application.
        VkApplicationInfo appInfo {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Pong";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_2;

        // Configuration parameters for Vulkan instance creation.
        VkInstanceCreateInfo createInfo {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        // Set the extensions in the configuration struct.
        createInfo.enabledExtensionCount = renderer->extensionCount;
        createInfo.ppEnabledExtensionNames = renderer->extensions;

        // If we want to see DEBUG messages from instance creation, we need to manually create a new
        // debug messenger that can be used in the function.
        VkDebugUtilsMessengerCreateInfoEXT debugInfo{};

        // Only enable the layer names in Vulkan if we're using validationlayers
        if (enableValidationLayers) {
            createInfo.ppEnabledLayerNames = renderer->validationLayers;
            // populate the messenger with our callback data.
            populateDebugMessengerCreateInfo(debugInfo);
            createInfo.enabledLayerCount = renderer->validationLayerCount;
            // pNext is an extension field. This is where pointers to callbacks and
            // messengers can be stored.
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugInfo;
        } else {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        if (vkCreateInstance(&createInfo, nullptr, &renderer->instance) != VK_SUCCESS) {
            return Status::INITIALIZATION_FAILURE;
        }

        return Status::SUCCESS;
    }

    Status checkValidationLayerSupport(
        uint32_t layerCount,
        VkLayerProperties* layerProperties,
        const char** validationLayers,
        uint32_t validationLayersCount
    ) {
        // Get the layer names themselves USING the count variable.
        vkEnumerateInstanceLayerProperties(&layerCount, layerProperties);
        uint8_t foundCount = 0;

        // Search to see whether the validation layers requested are supported by Vulkan.
        for (size_t i = 0; i < validationLayersCount; i++) {
            for (size_t j = 0; j < layerCount; j++) {
                if (strcmp(validationLayers[i], layerProperties[j].layerName) == 0) {
                    foundCount++;
                }
            }
        }

        // Throw an error and end the program if the validation layers aren't found.
        if (foundCount != validationLayersCount) {
            return Status::FAILURE;
        }

        INFO("Requested Validation layers exist!");

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

    Status initialiseDebugUtilsMessenger(VkInstance instance, VkDebugUtilsMessengerEXT* debugMessenger) {

        VkDebugUtilsMessengerCreateInfoEXT messengerInfo{};
        // Fill the struct with configuration data.
        populateDebugMessengerCreateInfo(messengerInfo);
        // Now we need to create the messenger and ensure it was successful:
        if (createDebugUtilsMessengerEXT(instance, &messengerInfo, nullptr, debugMessenger)
            != VK_SUCCESS) {
            // Throw an error and then stop execution if we weren't able to create the messenger
            return Status::INITIALIZATION_FAILURE;
        }

        return Status::SUCCESS;
    }

    // TODO: Move Debug Messenger code to a different file
    // Simply accepts a reference to a messenger config struct and fills it with data.
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& messengerInfo) {
        messengerInfo = {};
        // Specify the type of struct that's being populated
        messengerInfo.sType =           VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        // You can specify the severity levels of logs that this struct will accept.
        messengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                                        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                                        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        // You can also limit the actual message type to specific types.
        messengerInfo.messageType =     VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                                        | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                                        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        // Finally, you can store the actual debug callback in the struct
        messengerInfo.pfnUserCallback = debugCallback;
        messengerInfo.pUserData =       nullptr;
    }

    Status createWindowSurface(VkInstance instance, GLFWwindow* window, VkSurfaceKHR* surface) {
        if (glfwCreateWindowSurface(instance, window, nullptr,
                                    surface) != VK_SUCCESS) {
            ERROR("Failed to create window surface!");
            return Status::INITIALIZATION_FAILURE;
        }

        return Status::SUCCESS;
    }

    Status createPhysicalDevice(Renderer* renderer) {

        renderer->deviceData.physicalDevice = VK_NULL_HANDLE;

        // Begin by querying which devices are supported by Vulkan on this machine.
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(renderer->instance, &deviceCount, nullptr);

        if (deviceCount == 0) {
            ERROR("Failed to find GPUs that support Vulkan!");
            return Status::FAILURE;
        }

        // Now we query which specific devices we have
        VkPhysicalDevice devices[deviceCount];
        vkEnumeratePhysicalDevices(renderer->instance, &deviceCount, devices);

        Status status = Status::FAILURE;

        // Now search for the GPU we want.
        for (size_t i = 0; i < deviceCount; i++) {

            // We need a device that has the queue families that we need.
            VulkanUtils::QueueFamilyIndices indices = VulkanUtils::findQueueFamilies(devices[i],
                renderer->deviceData.surface);

            // Now we query all available extensions on this device
            uint32_t extensionCount = 0;
            vkEnumerateDeviceExtensionProperties(devices[i], nullptr, &extensionCount, nullptr);

            VkExtensionProperties availableExtensions[extensionCount];
            vkEnumerateDeviceExtensionProperties(devices[i], nullptr, &extensionCount, availableExtensions);

            // Check if our device has all our required extensions
            size_t found = 0;
            for (size_t i = 0; i < renderer->deviceExtensionCount; i++) {
                for (size_t j = 0; j < extensionCount; j++) {
                    if (strcmp(renderer->deviceExtensions[i], availableExtensions[j].extensionName) == 0) {
                        found++;
                    }
                }
            }

            bool extensionsSupported = (found == renderer->extensionCount);
            bool swapchainAdequate = false;

            if (extensionsSupported) {
                // Get the swapchain details
                VulkanUtils::SwapchainSupportDetails supportDetails =
                    VulkanUtils::querySwapchainSupport(devices[i], renderer->deviceData.surface);
                // Make sure that we have at least one supported format and one supported presentation mode.
                swapchainAdequate = !supportDetails.formats.empty() && !supportDetails.presentModes.empty();
            }

            bool is_supported = (
                indices.graphicsFamily.has_value()
                && indices.presentFamily.has_value())
                // If all available extensions were 'ticked off' the set then we know we have all
                // required extensions.
                && extensionsSupported
                && swapchainAdequate;
            
            // If the device has all our required extensions and has a valid swapchain and has our 
            // required queue families, then we can use that device for rendering!
            if (is_supported) {
                renderer->deviceData.physicalDevice = devices[i];
                status = Status::SUCCESS;
                break;
            }
        }

        return status;
    }

    void loadDefaultValidationLayers(Renderer* renderer) {

        const char* validationLayers[] = {
            "VK_LAYER_KHRONOS_validation"
        };

        renderer->validationLayers = validationLayers;
        renderer->validationLayerCount = 1;
    }

    Status loadCustomValidationLayers(Renderer* renderer, const char** validationLayers, uint32_t validationLayersCount) {

        Status status = Status::FAILURE;
        if (validationLayers != nullptr && validationLayersCount != 0) {
            renderer->validationLayers = validationLayers;
            renderer->validationLayerCount = validationLayersCount;
            status = Status::SUCCESS;
        } else {
            ERROR("Unable to load in validation layers! Invalid validation layers have been provided");
        }

        return status;
    }

    void loadDefaultDeviceExtensions(Renderer* renderer) {

        const char* deviceExtensions[] = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        renderer->deviceExtensions = deviceExtensions;
        renderer->deviceExtensionCount = 1;
    }

    Status loadCustomDeviceExtensions(Renderer* renderer, const char** extensions, uint32_t extensionCount) {

        Status status = Status::FAILURE;

        if (extensions != nullptr && extensionCount > 0) {
            renderer->deviceExtensions = extensions;
            renderer->deviceExtensionCount = extensionCount;
            status = Status::SUCCESS;
        }

        return status;
    }
}