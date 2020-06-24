#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <optional>
#include "logger.h"
#include <string>
#include <set>

#define PONG_FATAL_ERROR(...) ERROR(__VA_ARGS__); return EXIT_FAILURE

// Const variables to determine initial window height ad 
const int WINDOW_HEIGHT = 600;
const int WINDOW_WIDTH = 800;

// An array to specify the vaidation layers we want to use.
const char* validationLayers[] = {
    "VK_LAYER_KHRONOS_validation"
};

const char* deviceExtensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

// Number of layers we want to use (the number of values in the validationLayers array)
// REMEMBER TO UPDATE THIS IF NEW LAYERS ARE ADDED!
const uint32_t validationLayersCount = 1;

const uint32_t deviceExtensionsCount = 1;

// Only enable validation layers when the program is run in DEBUG mode.
#ifdef DEBUG
    const bool enableValidationLayers = true;
#else
    const bool enableValidationLayers = false;
#endif

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
};

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
    // A struct for storing the index of the queue family that the device will be using
    QueueFamilyIndices indices;

    // Again, get the queue families that the device uses.
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    VkQueueFamilyProperties queueFamilies[queueFamilyCount];

    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

    // Now that we know the families, we can now assess the suitability of this device.
    for (size_t i = 0; i < queueFamilyCount; i++) {
        // We search for a flag which specifies that the queue supports graphics operations.
        // This is specified with the VK_QUEUE_GRAPHICS_BIT flag.
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if (presentSupport) {
            indices.presentFamily = i;
        }
    }

    return indices;
}


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
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

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
    messengerInfo.pUserData = nullptr;
}

// Similar to messenger creation - destroying the messenger also requires the calling of
// an unloaded message. We need to do the same thing as before and load the method from memory.
// In this case we want to get a method for cleaning up messenger memory.
static void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger
    , const VkAllocationCallbacks* pAllocator) {
    // Load the method from memory
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
        instance, "vkDestroyDebugUtilsMessengerEXT");

    if (func != nullptr) {
        // Call the method if it was returned successfully.
        func(instance, debugMessenger, pAllocator);
    }
}

int main() {  

    // -------------------- INITIALISE WINDOW --------------------------
    initLogger();

    // Initialise GLFW
    glfwInit();

    // Set flags for the window (set it to not use GLFW API and 
    // set it to not resize)
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // Actually make the window
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Pong", nullptr, nullptr);

    INFO("Created GLFW window");

    // ========================= VULKAN SETUP ================================

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
            PONG_FATAL_ERROR("Requested validation layers are not available!");
        }

        INFO("Requested Validation layers exist!");
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
    INFO("Checking Extensions: ");

    std::string extensionStr = "\n";
    for (size_t i = 0; i < vulkanExtensionCount; i++) {
        std::string str = vkExtensions[i].extensionName;
        extensionStr += '\t' + str + '\n';
    }
    INFO(extensionStr);

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
        INFO("GLFW extensions are supported by Vulkan!");
    } else {
        PONG_FATAL_ERROR("GLFW extensions are NOT supported by Vulkan!");
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

    // If we want to see DEBUG messages from instance creation, we need to manually create a new
    // debug messenger that can be used in the function.
    VkDebugUtilsMessengerCreateInfoEXT debugInfo{};

    // Only enable the layer names in Vulkan if we're using validationlayers
    if (enableValidationLayers) {
        createInfo.ppEnabledLayerNames = validationLayers;
        // populate the messenger with our callback data.
        populateDebugMessengerCreateInfo(debugInfo);
        createInfo.enabledLayerCount = validationLayersCount;
        // pNext is an extension field. This is where pointers to callbacks and 
        // messengers can be stored.
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugInfo;
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        PONG_FATAL_ERROR("FAILED TO CREATE VULKAN INSTANCE!");
    }

        // ---------------- VALIDATION LAYER LOG MESSAGES -------------------

    // Struct for holding the debug callback
    VkDebugUtilsMessengerEXT debugMessenger;

    // We only care about debug messages if we're using validation layers. 
    if (enableValidationLayers) {
        // Messengers also have a configuration struct that needs to be filled in. 
        VkDebugUtilsMessengerCreateInfoEXT messengerInfo{};
        // Fill the struct with configuration data.
        populateDebugMessengerCreateInfo(messengerInfo);
        // Now we need to create the messenger and ensure it was successful:
        if (createDebugUtilsMessengerEXT(instance, &messengerInfo, nullptr, &debugMessenger)
        != VK_SUCCESS) {
            // Throw an error and then stop execution if we weren't able to create the messenger
            PONG_FATAL_ERROR("Failed to set up DEBUG messenger!");
        }
    }

    // --------------- WINDOW SYSTEM INTEGRATION SETUP ------------------

    // Vulkan doesn't handle integrating with window systems automatically. We need
    // to manually set this up wth our windowing system (which is GLFW).

    // Create a surface extension object.
    VkSurfaceKHR surface;

    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        PONG_FATAL_ERROR("Failed to create window surface!");
    }

    // ------------------- PHYSICAL DEVICE SETUP ------------------------

    // Vulkan requires us to specify the device that we'll be using for our rendering. 
    // Generally this means the GPUs that are available to us. 

    // The device struct - used to hold the physical device data.
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    // Following a similar pattern as before - we need to query for the number of supported 
    // devices and get the actual counts.
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    // Make sure at least one device is supported. 
    if (deviceCount == 0) {
        PONG_FATAL_ERROR("Failed to find GPUs that support Vulkan!");
    }

    // Get the actual device details.
    VkPhysicalDevice devices[deviceCount];
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices);

    // Now we need to actually find a device to use. This can be done using a number of
    // methods. One such method is to assign a rating to each GPU and pick the best rated one. 
    // In this case, since this is a tutorial, we'll just take the first GPU we find. 
    for (size_t i = 0; i < deviceCount; i++) {
        VkPhysicalDevice device = devices[i];

        // Now we need to actually find the exact queue families that this device uses. 
        // In Vulkan, all commands are executed in a queue of similar types. 
        // We need to find exactly which queues this device uses so we can use them
        // to execute commands. 
        
        QueueFamilyIndices indices = findQueueFamilies(device, surface);

        // Now we need to check whether our physical device supports drawing images to the screen.

        // Get the number of extensions
        uint32_t extensionCount = 0;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        VkExtensionProperties availableExtensions[extensionCount];
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions);

        // Store the unique extensions in a set so we can quickly remove duplicates.
        std::set<std::string> requiredExtensions(deviceExtensions+0, deviceExtensions+deviceExtensionsCount);

        for (size_t i = 0; i < extensionCount; i++) {
            // If all required extensions are available from our GPU then we know we have full 
            // Vulkan support!
            requiredExtensions.erase(availableExtensions[i].extensionName);
        }

        bool is_supported = (
            indices.graphicsFamily.has_value() 
            && indices.presentFamily.has_value()) 
            // If all available extensions were 'ticked off' the set then we know we have all
            // required extensions.
            && requiredExtensions.empty();

        // Finally, we check if the current device has a valid graphics queue. If so then we just
        // use it (at least for now).
        // TODO: Add some sort of function to get a graphics card that's most suitable for us.
        if (is_supported) {
            physicalDevice = device;
            break;
        }
    }

    // Finally, we just need to make sure that an actual valid device was returned to us from 
    // our search.
    if (physicalDevice == VK_NULL_HANDLE) {
        PONG_FATAL_ERROR("Failed to find a suitable GPU for Vulkan!");
    }

    // --------------------- LOGICAL DEVICE SETUP -----------------------

    // Once a physical device has been set up we then need to setup a logical device.
    // A logical device interfaces with the physical device and is used to actually execute
    // commands to the hardware. 

    // We start by creating a device.
    VkDevice device;

    // Get the queue families from the physical device that we got previously. 
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);

    // TODO: There's probably a way to do this with an array rather than a vector + set.
    std::vector<VkDeviceQueueCreateInfo> createInfos;
    // Create a set so we can store queue families by their unique index (stops us from 
    // using the same index twice.)
    std::set<uint32_t> uniqueFamilies = {
        indices.graphicsFamily.value(), indices.presentFamily.value()};

    // Set a priority for these queues - for now we'll set to maximum.
    float priority = 1.0f;

    // Create a queue creation struct for each unique queue family we have
    for (auto& queueFamily : uniqueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &priority;
        createInfos.push_back(queueCreateInfo);
    }

    // Leave this empty for now - can add things later when we need.
    VkPhysicalDeviceFeatures deviceFeatures{};

    // Now we need to actually configure the logical device (note that it uses the queue info
    // and the device features we defined earlier).
    VkDeviceCreateInfo logicalDeviceInfo{};
    logicalDeviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    logicalDeviceInfo.pQueueCreateInfos = createInfos.data();
    logicalDeviceInfo.queueCreateInfoCount = createInfos.size();
    logicalDeviceInfo.pEnabledFeatures = &deviceFeatures;
    logicalDeviceInfo.enabledExtensionCount = 0;

    // Now we create the logical device using the data we've accumulated thus far. 
    if (vkCreateDevice(physicalDevice, &logicalDeviceInfo, nullptr, &device) != VK_SUCCESS) {
        PONG_FATAL_ERROR("Failed to create logical device!");
    }

    // Now we just need to create the queue which will be used for our commands.

    // Create the queue struct.
    VkQueue graphicsQueue;

    // Instantiate the presentation queue.
    VkQueue presentQueue;

    // Create the queue using the struct and logical device we created eariler.
    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    // Create the presentation queue using the struct.
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);

    // ======================= END OF SETUP =============================

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

    // Destroy window surface
    vkDestroySurfaceKHR(instance, surface, nullptr);

    // Destroy logical device
    vkDestroyDevice(device, nullptr);

    // Vulkan cleanup
    vkDestroyInstance(instance, nullptr);
    
    // GLFW cleanup
    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS; 
} 