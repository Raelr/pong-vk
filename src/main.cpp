#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <vector>

// Const variables to determine initial window height ad 
const int WINDOW_HEIGHT = 600;
const int WINDOW_WIDTH = 800;

// An array to specify the vaidation layers we want to use.
const char* validationLayers[] = {
    "VK_LAYER_KHRONOS_validation"
};

// Number of layers we want to use (the number of values in the validationLayers array)
// REMEMBER TO UPDATE THIS IF NEW LAYERS ARE ADDED!
const uint32_t validationLayersCount = 1;

// Only enable validation layers when the program is run in DEBUG mode.
#ifdef DEBUG
    const bool enableValidationLayers = true;
#else
    const bool enableValidationLayers = false;
#endif

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType, 
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
    
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

static VkResult createDebugUtilsMessengerEXT(VkInstance instance, 
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger) {

    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
        instance, "vkCreateDebugUtilsMessengerEXT");

    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

static void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger
    , const VkAllocationCallbacks* pAllocator) {

    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
        instance, "vkDestroyDebugUtilsMessengerEXT");

    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}


int main() {  

    // -------------------- INITIALISE WINDOW --------------------------

    // Initialise GLFW
    glfwInit();

    // Set flags for the window (set it to not use GLFW API and 
    // set it to not resize)
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // Actually make the window
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Pong", nullptr, nullptr);

    // ------------------ VALIDATION LAYER SUPPORT CHECKING ------------------

    uint32_t layerCount = 0;

    // Get the number of layers available for vulkan.
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    VkLayerProperties layerProps[layerCount];

    if (enableValidationLayers) {
        
        // Get the layer names themselves USING the count variable.
        vkEnumerateInstanceLayerProperties(&layerCount, layerProps);

        uint8_t foundCount = 0;

        // Search to see whether the validation layers requested are supported by Vulkan.
        for (size_t i = 0; i < validationLayersCount; i++) {
            for (size_t j = 0; j < layerCount; j++) {
                if (strcmp(validationLayers[i], layerProps[j].layerName) == 0) {
                    foundCount++;
                }
            }
        }

        // Throw an error and end the program if the validation layers aren't found. 
        if (!(foundCount == validationLayersCount)) {
            std::cout << "Requested validation layers are not available!" << std::endl;
            return EXIT_FAILURE;
        }

        std::cout << "Requested Validation layers exist!" << std::endl;
    }

    // ------------------ INITIALISE VULKAN INSTANCE ---------------------

    // Initialise the Vulkan instance struct. 
    VkInstance instance; 

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

    // -------------------- VULKAN EXTENSION VERIFICATION -------------------------------

    // Get all supported extensions by Vulkan:
    uint32_t vulkanExtensionCount = 0;

    // This instance of the method will return the number of supported extensions. 
    vkEnumerateInstanceExtensionProperties(nullptr, &vulkanExtensionCount, nullptr);

    VkExtensionProperties vkExtensions[vulkanExtensionCount];

    // This instance of the method will return the exact extensions supported
    vkEnumerateInstanceExtensionProperties(nullptr, &vulkanExtensionCount, vkExtensions);

    // Print out and display all extensions.
    std::cout << "Checking extensions..." << std::endl;

    for (size_t i = 0; i < vulkanExtensionCount; i++) {
        std::cout << '\t' << vkExtensions[i].extensionName << std::endl;
    }

    // Now we need to get the extensions required by GLFW in order for it to work with Vulkan.
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;

    // Get the names of all glfw extensions.
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    size_t foundCount = 0;
    // Check if the GLFW extensions match the supported extensions by Vulkan.
    for (size_t i = 0; i < glfwExtensionCount; i++) {
        for (size_t j = 0; j < vulkanExtensionCount; j++) {
            if (strcmp(vkExtensions[j].extensionName, glfwExtensions[i]) == 0) {
                foundCount++;
            }
        }
    }

    // Check if all GLFW extensions are supported by Vulkan.
    if (foundCount == glfwExtensionCount) {
        std::cout << "GLFW extensions are supported by Vulkan!" << std::endl;
    } else {
        std::cout << "GLFW extensions are NOT supported by Vulkan!";
        return EXIT_FAILURE;
    }

    // A combined vector for all extensions.
    // TODO: Maybe find an alternative to the vector class? Not a priority at the moment.
    std::vector<const char*> extensions(glfwExtensions + 0, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        // Push back the debug utils extension for Vulkan ONLY if validation layers are enabled
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    // --------------------- CONTINUE INSTANCE CREATION ------------------------

    // Set the extensions in the configuration struct. 
    createInfo.enabledExtensionCount = extensions.size();
    createInfo.ppEnabledExtensionNames = extensions.data();

    // Only enable the layer names in Vulkan if we're using validationlayers
    if (enableValidationLayers) {
        createInfo.ppEnabledLayerNames = validationLayers;
        createInfo.enabledLayerCount = validationLayersCount;
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        printf("FAILED TO CREATE VULKAN INSTANCE!");
        return EXIT_FAILURE;
    }

    // ---------------- VALIDATION LAYER LOG MESSAGES -------------------

    VkDebugUtilsMessengerEXT debugMessenger;

    if (enableValidationLayers) {
        
        VkDebugUtilsMessengerCreateInfoEXT messengerInfo{};
        messengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        messengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT 
                                        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT 
                                        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        messengerInfo.messageType =     VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT 
                                        | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                                        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        messengerInfo.pfnUserCallback = debugCallback;
        messengerInfo.pUserData = nullptr;

        if (createDebugUtilsMessengerEXT(instance, &messengerInfo, nullptr, &debugMessenger)
            != VK_SUCCESS) {
            
            std::cout << "Failed to set up DEBUG messenger!" << std::endl;
            return EXIT_FAILURE;
        }
    }

    // ------------------------- MAIN LOOP ------------------------------

    // Start the loop and only stop when a close event happens.
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    // --------------------------- CLEANUP ------------------------------

    // Cleaning up memory

    if(enableValidationLayers) {
        destroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }

    // Vulkan cleanup
    vkDestroyInstance(instance, nullptr);

    // GLFW cleanup
    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS; 
} 