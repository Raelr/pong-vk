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
#include <cstdint>
#include "utils.h"

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

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
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

// Checks whether our physical device has swapchain support. Returned a struct with multiple
// configuration details
SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
    // instantiate a struct to store swapchain details.
    SwapChainSupportDetails details;

    // Now follow a familiar pattern and query all the support details from Vulkan...
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());

    // Using a vector for the utility functions - statically resize the data within it to hold 
    // the data we need.
    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    // Same as above ^
    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, 
            details.presentModes.data());
    }

    // Return the details we need
    return details;
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
static void destroyDebugUtilsMessengerEXT(VkInstance instance, 
     VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    // Load the method from memory
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
        instance, "vkDestroyDebugUtilsMessengerEXT");

    if (func != nullptr) {
        // Call the method if it was returned successfully.
        func(instance, debugMessenger, pAllocator);
    }
}

// All shaders must be wrapped in a shader module. This is a helper function
// for wrapping the shader. 
VkShaderModule createShaderModule(FileContents buffer, VkDevice &device) {
    // As is usually the case, we pass the config information to an info struct
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    // Add in the buffer size
    createInfo.codeSize = buffer.fileSize;
    // Add in the bytecode itself. The bytecode needs to be submitted in bytes, so
    // we need to cast this to a 32-bit unsigned integer to make this work.
    createInfo.pCode = reinterpret_cast<const uint32_t*>(buffer.p_byteCode);
    // Create the shader
    VkShaderModule shader;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shader) != VK_SUCCESS) {
        // TODO: Find a better method to propagate errors.
        printf("Unsable to create shader module!\n");
    }
    // Return the shader
    return shader;
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
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Pong", 
        nullptr, nullptr);

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

        bool extensionsSupported = requiredExtensions.empty();
        
        // Check if we have support for swapchains!
        bool swapChainAdequate = false;

        // Only check if we have supported extensions.
        if (extensionsSupported) {
            // Get the swapchain details
            SwapChainSupportDetails supportDetails = querySwapChainSupport(device, surface);
            // Make sure that we have at least one supported format and one supported presentation mode.
            swapChainAdequate = !supportDetails.formats.empty() && !supportDetails.presentModes.empty();
        }

        bool is_supported = (
            indices.graphicsFamily.has_value() 
            && indices.presentFamily.has_value()) 
            // If all available extensions were 'ticked off' the set then we know we have all
            // required extensions.
            && extensionsSupported
            && swapChainAdequate;

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
    logicalDeviceInfo.enabledExtensionCount = deviceExtensionsCount;
    logicalDeviceInfo.ppEnabledExtensionNames = deviceExtensions;

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

    // --------------------- SWAP CHAIN CREATION ------------------------

    // Get the surface format for the swapchain:
    SwapChainSupportDetails swapChainDetails= querySwapChainSupport(physicalDevice, surface);

    // We want to find three settings for our swapchain:
    // 1. We want to find the surface format (color depth).
    // 2. Presentation Mode (conditions for 'swapping' images to the screen - kinda like vSync).
    // 3. Swap Extent (resolution of images in the swapchain)

    // First lets get the format, we'll set it to the first format in the list to start.
    VkSurfaceFormatKHR chosenformat = swapChainDetails.formats[0];

    // The Format struct contains two variables that should be set:
    // 1. Format - The color channels and types used by the Vulkan.
    // 2. Colorspace - Checks of the SRGB colo space is supported or not

    for (auto& format : swapChainDetails.formats) {
        // In our case we'll be looking for the SRGB color space since it results in
        // more accurate percieved colors. 
        // We'll also store each color channel as an 8-bit integer (resulting in 32 bits total).
        // Format is also going to be: B G R A - in that order.
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB 
            && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            chosenformat = format;
            INFO("Found SRGB channel for the rendering format.");
            break;
        } // TODO: If this fails then might be good to rank other formats depending on their desirability. 
    }

    // The present mode details the conditions for showing images on screen, this comes with a few
    // settings:

    // 1. VK_PRESENT_MODE_IMMEDIATE_KHR - images are transferred to screen right away (might result
    // in screen tear).
    // 2. VK_PRESENT_MODE_FIFO_KHR - Similar to Vsync (uses a queue to process images). Essentially
    // processes images in the queue in batches. Application is also blocked when the queue is full.
    // 3. VK_PRESENT_MODE_FIFO_RELAXED_KHR - Sends an image to screen if the queue is empty.
    // 4. VK_PRESENT_MODE_MAILBOX_KHR - Same as 2, except that it doesn't block when the queue
    // is full. Older images are simply replaced with newer ones. This results in triple buffering
    // (which avoids screen tear and latency issues caused by standard Vsync).

    // Now we search for available present modes:
    
    // Set present mode to Vsync as default.
    VkPresentModeKHR chosenPresentMode = VK_PRESENT_MODE_FIFO_KHR;

    for (auto& presentMode: swapChainDetails.presentModes) {
        // If we can, we should try and get triple buffering as a configuration.
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            chosenPresentMode = presentMode;
            INFO("Triple buffering enabled for Present mode.");
            break;
        }
    }
    
    // Finally, we need to set the swap extent. The Swap extent is the resolution of the images in
    // the swapchain. Generally speaking, its recommended to set this to the window width/height.
    VkExtent2D chosenExtent = {WINDOW_WIDTH, WINDOW_HEIGHT};

    // Make sure to clamp both the x and y values between the min and max
    chosenExtent.width = std::clamp(chosenExtent.width, 
        swapChainDetails.capabilities.minImageExtent.width, 
        swapChainDetails.capabilities.maxImageExtent.width);

    chosenExtent.height = std::clamp(chosenExtent.height, 
        swapChainDetails.capabilities.minImageExtent.height, 
        swapChainDetails.capabilities.maxImageExtent.height);

    // If the extent is already the maximum of t_uint32 then we simply set the extent
    // to the default returned by our physical device. 
    if (swapChainDetails.capabilities.currentExtent.width != UINT32_MAX) {
        chosenExtent = swapChainDetails.capabilities.currentExtent;
    }

    INFO("Device extent has been set to: [ " 
        + std::to_string(chosenExtent.width) + ", " + std::to_string(chosenExtent.height) + " ]");

    // --------------------- CREATE SWAPCHAIN ---------------------------

    // Lets get the number of image that we'd like the Swapchain to have:


    // Start the loop and only stop when a close event happens. We'll get the minimum 
    // images to start with and add 1 to the end (just to allow one more image in!)
    uint32_t imageCount = swapChainDetails.capabilities.minImageCount + 1;

    // Make sure that we aren't assigning more images to the queue than it can handle.
    // NOTE: A max imagecount of 0 implies that there is no max.
    if (swapChainDetails.capabilities.maxImageCount > 0 
        && imageCount > swapChainDetails.capabilities.maxImageCount) {
        
        imageCount = swapChainDetails.capabilities.maxImageCount;
    }

    // Create the swapchain creation struct and assign all the previous values to it.
    VkSwapchainCreateInfoKHR swapchainCreateInfo {};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = surface;
    swapchainCreateInfo.minImageCount = imageCount;
    swapchainCreateInfo.imageFormat = chosenformat.format;
    swapchainCreateInfo.imageColorSpace = chosenformat.colorSpace;
    swapchainCreateInfo.imageExtent = chosenExtent;
    // Specifies tbe number of layers the image will consist of. 
    swapchainCreateInfo.imageArrayLayers = 1;
    // Specifies the kind of operations the images in the swapchain will be used for.
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t QueueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily) {
        // Images can be owned by multiple queue families without explicit changes in ownership.
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCreateInfo.queueFamilyIndexCount = 2;
        swapchainCreateInfo.pQueueFamilyIndices = QueueFamilyIndices;
    } else {
        // This specifies that the image can only be owned by a single queue family at a time. 
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCreateInfo.queueFamilyIndexCount = 0;
        swapchainCreateInfo.pQueueFamilyIndices = nullptr;
    }

    // Can be used to specify a transform that all images in the swapchain will follow. 
    // In this case we'll just set it to the default.
    swapchainCreateInfo.preTransform = swapChainDetails.capabilities.currentTransform;

    // Specifies if the alpha channel should be used for blending with other windows.
    // We'll be ifnoring this for now.
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    swapchainCreateInfo.presentMode = chosenPresentMode;

    // Vulkan swapchains can become irrelevant when certain details are met (such as if the 
    // screen is resized). In this case we need to specify the old swapchain.
    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    // Now we can create the swapchain:
    VkSwapchainKHR swapchain;

    // Create the swapchain and stop the program if the creation fails.
    if (vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain) != VK_SUCCESS) {
        PONG_FATAL_ERROR("Failed to create swapchain!");
    }

    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);

    VkImage swapchainImages[imageCount];

    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages);

    VkFormat swapchainFormat;

    swapchainFormat = chosenformat.format;

    VkExtent2D swapChainExtent = chosenExtent;

    // --------------------- Create Image Views -------------------------

    // A VkImageView object is required to use any Images in Vulkan.
    // A view describes how to access an image and which part of an image should be accessed. 

    // Store all images in an array.
    VkImageView swapChainImageViews[imageCount];

    for (size_t i = 0; i < imageCount; i++) {
        // We need to create a view for every image that we stored for the swapChain.
        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = swapchainImages[i];
        // Allows you to specify whether the image will be viewed as a 1D, 2D, or 3D texture.
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = swapchainFormat;
        // Components field allow us to swizzle values around (force them to assume certain
        // values).
        // In this case we'll set the components to their default values.
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        // The subresourceRange field describes an image's purpose. 
        // In our case our images will be used as color targets with no mipmapping levels
        // or layers. 
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        // Create the image view and store it in the swapChainImageViews array.
        if (vkCreateImageView(device, &imageViewCreateInfo, nullptr, &swapChainImageViews[i]) 
            != VK_SUCCESS) {
            PONG_FATAL_ERROR("Failed to create image view!");
        }
    }

    // ------------------------ RENDER PASS -----------------------------
    
    // In Vulkan, a render pass represents all information that our framebuffers. 
    // This is primarily done by specifying the framebuffer attachments. 

    // To do this, we need to specify how many color and depth buffers there 
    // will be, how amny samples to use for them, and how these contents will
    // be handled in the rendering operations. This info is all wrapped in a 
    // render pass object. 

    // In this case, we'll have a single color buffer attachment which is 
    // represented by one of the images in our swapchain:

    VkAttachmentDescription colorAttachment{};
    // Set this to the imageFormat we chose earlier for the swapchain. 
    colorAttachment.format = swapchainFormat;
    // If we were using multi-sampling we'd set this to more than one bit. 
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    // loadOp determines what happens to the contents of an attachment before
    // rendering. It has the following options:
    // 1. VK_ATTACHMENT_LOAD_OP_LOAD - preserve existing contents of the attachment. 
    // 2. VK_ATTACHMENT_LOAD_OP_CLEAR - clears values to a constant at the start. 
    // 3. VK_ATTACHMENT_LOAD_OP_DONT_CARE - existing contents are undefined. 
    //                                      We don't care about them.
    // In our case we're going to clear the buffer to black before drawing a 
    // new frame. 
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    // storeOp defines what will happen AFTER rendering:
    // 1. VK_ATTACHMENT_STORE_OP_STORE - render contents will be stored in memory. 
    // 2. VK_ATTACHMENT_STORE_OP_DONT_CARE - render contents will be undefined after
    //                                       rendering. 
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    // For now we aren't going to use stencil data, so we'll let that data be 
    // undefined. 
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; 
    // The layout defines how an image is formatted in memory. The most common
    // layouts include:
    // 1. VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL - Images are used as color attachments. 
    // 2. VK_IMAGE_LAYOUT_PRESENT_SRC_KHR - Images represented in a swapchain.
    // 3. VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL - Images used as a destination for a memory copy
    //                                           operation.
    // Essentially, images need to be transitioned into whatever layout suits the 
    // operation they'll be involved in next. 
    
    // This field specifies the layout the image is in before the render pass begins. 
    // In our case we don't really care what layout it is at the start.
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    // Sine the image needs to be ready for representation in the swapchain, we 
    // make sure the final layout of this image is suitable for that: 
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // Next, we need to specify any subpasses that will be used by the framebuffer. 
    // Every render pass is made up of subpasses. These subpasses are then bundled
    // into a single pass where Vulkan is then re-order them to conserve memory 
    // bandwidth.
    
    // In our case we'll just use one subpass since we want to just render a triangle. 
    // Each subpass uses a reference to a color attachment, these are represented
    // by a VkAttachmentReference struct. 
    VkAttachmentReference colorAttachmentRef{};
    // This field specifies which attachment to reference. This is usually done
    // by index since our color descriptions are typically stored in one. Since 
    // we only have a single attachment we specify the index as 0.
    colorAttachmentRef.attachment = 0; 
    // This field specifies which layout we want the attachment to have during 
    // the subpass. In our case, we want this to act as a color buffer and 
    // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL will provide the most optimal
    // performance for that function. 
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Now that we've specified a reference, we can create the subpass itself:
    VkSubpassDescription subpass{};
    // Here we specify that this subpass is used for graphics rendering.
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    // Specify the count of color attachments that we're using here.
    subpass.colorAttachmentCount = 1;
    // Pass the attachments reference into the struct. 
    subpass.pColorAttachments = &colorAttachmentRef;

    // A subpass can also store the following attachments:
    // 1. pInputAttachments - attachments that are read from a shader.
    // 2. pResolveAttachments - attachments used for multisampling color attachments.
    // 3. pDepthStencilAttachment - attachment for depth and stencil data.
    // 4. pPreserveAttachments - attachments not used by this subpass, but which 
    //                           need their data to be preserved.

    // Now that we have a basic subpass defined, we can finally create our 
    // renderpass! 
   
    // Create the render pass struct.
    VkRenderPass renderPass;
    
    // Create the render pass creation struct and pass all the previous data 
    // into it.
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    
    // Create the render pass object using the info we created previously.
    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) 
            != VK_SUCCESS) {
        PONG_FATAL_ERROR("Failed to create render pass!");
    }

    // --------------------- GRAPHICS PIPELINE --------------------------
    
    // Vulkan requires that you define your own graphics pipelines when you
    // want to use different combinations of shaders. This is because the 
    // graphics pipeline in Vulkan is almost completely immutable. This means
    // that the pipeline can be very well optimised (but will also require
    // a complete rewrite if you need anything different).
    //
    // In this case we'll set up a vertex and fragment shader so we can get
    // a triangle showing on screen.
    
    // To begin, lets load in our vertex and fragment shaders (in 
    // machine-readable bytecode)
    auto vert = readFile("src/shaders/vert.spv");
    auto frag = readFile("src/shaders/frag.spv");
    
    // Wrap the file contents in a shadermodule.
    VkShaderModule vertShaderModule = createShaderModule(vert, device);
    VkShaderModule fragShaderModule = createShaderModule(frag, device);
    
    // Shaders need to be manually added to a stage in the rendering 
    // pipeline. This is generally achieved using a VkPipelineShaderStageCreateInfo
    // struct.
    

    // TODO: Find a way to do this via a single function call?
    // Vertex shader create info:
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    // This variable tells Vulkan to insert this into the vertex shader stage
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    // Specify the shader module to be used.
    vertShaderStageInfo.module = vertShaderModule;
    // Specify the entrypoint to the shader (i.e: the main function)
    vertShaderStageInfo.pName = "main";
    
    // Frag shader create info:
    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    // This variable tells Vulkan to insert this into the fragment shader stage
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    // Specify the shader module to be used.
    fragShaderStageInfo.module = fragShaderModule;
    // Specify the entrypoint to the shader (i.e: the main function)
    fragShaderStageInfo.pName = "main";

    // Store the stage information in an array for now - will be used later.
    VkPipelineShaderStageCreateInfo shaderStages[] = {
        vertShaderStageInfo,
        fragShaderStageInfo
    };
    
    // ------------------------ PIPELINE STRUCTS ----------------------------
    
    // Create a struct for detailing how vertex data will be formatted in the
    // vertex shader.

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    // Describes details for loading vertex data.
    vertexInputInfo.pVertexBindingDescriptions = nullptr;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    // Describes details for loading vertex data.
    vertexInputInfo.pVertexAttributeDescriptions = nullptr;

    // Now define the input assembly. This defines the kind of geometry drawn
    // from the vertices and if primitive restart is enabled. 
    // Geometry (specified by the topology) member has the options:
    // VK_PRIMITIVE_TOPOLOGY_POINT_LIST - builds geometry from points of vertices. 
    // VK_PRIMITIVE_TOPOLOGY_LINE_LIST - line from every 2 vertices without reuse.
    // VK_PRIMITIVE_TOPOLOGY_LINE_STRIP - The end of every vertex line is used as
    //                                    the start vertex of the next line
    // VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST - Create a triangle from every 3 vertices
    //                                      without reuse.
    // VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP - the 2nd and 3rd vertex of every
    //                                        triangle are used as the first two
    //                                        vertices of the next.

    // For us, we'll use the TRIANGLE_LIST option to help us draw a triangle:
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Now, we need to create a viewport. A viewport defines the region of the 
    // framebuffer that the output will be rendered to. In this case the viewport
    // will range between (0,0) and (width, height):
    
    VkViewport viewPort{};
    viewPort.x = 0.0f;
    viewPort.y = 0.0f;
    viewPort.width = (float) chosenExtent.width;
    viewPort.height = (float) chosenExtent.height;
    viewPort.minDepth = 0.0f;
    viewPort.maxDepth = 1.0f;

    // Now we need to define scissors. Scissors define which regions the pixels
    // will actually be stored. These regions are generally specified as 
    // rectangles where anything in the rectangle is rendered whilst anything 
    // outside the window is discarded. 

    // In this case, we'll just draw it over the entire screen.
    VkRect2D scissor{};
    scissor.offset = {0,0};
    scissor.extent = chosenExtent;

    // Now, the scissor and Viewport need to be combined into a single struct.
    // This struct is known as a ViewportState: 
    
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    // This takes in an array of viewports. Some GPUs allow multiple viewports
    // to be specified, but this must be enabled as a device feature (in logical
    // device section)
    viewportState.pViewports = &viewPort;
    viewportState.scissorCount = 1;
    // Same as with viewports - this takes in an array of scissors. 
    viewportState.pScissors = &scissor;

    // Now that we have a viewport, we can start building out the rasterizer. 
    // The rasterizer takes in the geometry shaped by the shader's vertices 
    // and turns them into fragments. These fragments are then colored by the 
    // fragment shader. 
    // The rasterizer also performs face culling and depth testing. 
    
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    // This means that fragments below the near/far plane are clamped to the planes
    // as opposed to discarding them. This requires a GPU feature in order for it
    // to work. 
    rasterizer.depthClampEnable = VK_FALSE;
    // Checks if geometry should pass through the rasterizer stage. We only skip
    // the stage if this is set to VK_TRUE.
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    // Determines how fragments are generated for geometry. 
    // VK_POLYGON_MODE_FILL - fill the area of the polygon with fragments.
    // VK_POLYGON_MODE_LINE - Polygon edges are drawn as lines.
    // VK_POLYGON_MODE_POINT - Polygon vertices are drawn as points.
    //
    // NOTE: The use of anything other than FILL requires a GPU feature to be
    // enabled. 
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    // Describes the thickness of lines. Any lines thicker than 1.0f requires 
    // a GPU feature to be enabled. 
    rasterizer.lineWidth = 1.0f;
    // Determines the type of face-culling to use.
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    // Determines the vertex order for faces. Determines which are front-facing 
    // and which re back-facing. Can be clockwise or counter-clockwise. 
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    // Alters the depth values by adding a constant value or biasing depth 
    // values. Can be useful for shadow mapping. 
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;  // optional
    rasterizer.depthBiasClamp = 0.0f;           // optional
    rasterizer.depthBiasSlopeFactor = 0.0f;     // optional

    // We also need to specify a MultiSampling struct. Multisampling allows us
    // to create effects like anti-aliasing in our programs. 
    //
    // Multisampling works by combining the fragment shader results into of
    // multiple polygons and rasterize them to the same pixel. This usually 
    // occurs along edges where we get the most artifacting.
    
    // Enabling multisampling requires a GPU feauture to be enabled. 
    // For now we'll disable multi-sampling and revisit it at a later stage. 
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO; 
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;          // optional
    multisampling.pSampleMask = nullptr;            // optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // optional
    multisampling.alphaToOneEnable = VK_FALSE;      // optional

    // The next stage is to define how color blending works. The color blending 
    // stage happens once a fragment shader returns a color. This color is then
    // combined with a color that's already in the frame-buffer. 
    
    // Color blending can happen in one of two ways:
    // 1) Mix the old and new values to produce a final color. 
    // 2) Use a bitwise operation to combine the old and new colors. 

    // First, we need to create a BlendAttachmentState struct. This struct 
    // contains configuration per attached framebuffer:
    
    // We'll follow the first method of color blending:
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | 
        VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;   
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;     // optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;    // optional
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;                // optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;     // optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;    // optional
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;                // optional

    // Now we need to actually build the createInfo struct. 
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    // Set this to VK_TRUE to enable bitwise blending.
    colorBlending.logicOpEnable = VK_FALSE;
    // This is where the bitwise combination is set (if logicOpEnable is set to true)
    colorBlending.logicOp = VK_LOGIC_OP_COPY;           // optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;             // optional
    colorBlending.blendConstants[1] = 0.0f;             // optional
    colorBlending.blendConstants[2] = 0.0f;             // optional
    colorBlending.blendConstants[3] = 0.0f;             // optional

    // At this point it is possible to create a VkDynamicState struct. This will 
    // allow certain data to be modified without requiring the pipeline to be remade. 
    // We won't implement this now, but we might do so at a later stage. 
    
    // PLACEHOLDER: VkDynamicState state;
   
    // The next (and final) struct we need for now is to create a PipelineLayout
    // struct. The PipelineLayout struct details all the uniform values within
    // our shaders. This is required to be specified at the time of creation of
    // the pipeline.
    
    // First create an empty pipeline layout struct. 
    VkPipelineLayout pipelineLayout{};

    // Then instantiate the createInfo struct for that object:
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 0;
    pipelineLayoutCreateInfo.pSetLayouts = nullptr;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

    // Now create the pipeline layout using the usual method:
    if (vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout) 
            != VK_SUCCESS) {
    
        PONG_FATAL_ERROR("Failed to create pipeline layout");
    }

    // Finally, we can no create out graphics pipeline:
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    // Now we input all the structs that we defined previously into this one:
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState; 
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = nullptr;
    // Then we reference the layout struct. 
    pipelineInfo.layout = pipelineLayout;
    // Now we pass the renderPass into this.
    pipelineInfo.renderPass = renderPass;
    // We reference the subpass by index. 
    pipelineInfo.subpass = 0;
    // This field allows us to specify a pipeline that this one can be pased off. 
    // Since we need to re-create a pipeline whenever a property changes, Vulkan
    // allows us to specify an existing pipeline to act as a base for this one. 
    
    // In our case we'll provide no base pipeline since we're not going to be
    // changing the pipeline much.
    
    // NOTE: These fields are only used if we've specified VK_PIPELINE_CREATE_DERIVATIVE_BIT
    // as a flag to the struct. 

    // You can reference it with either a handle of an existing pipeline.
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;   // optional
    // Or you can reference that pipeline by index. 
    pipelineInfo.basePipelineIndex = -1;                // optional

    // Now we can create a pipeline!
    VkPipeline graphicsPipeline;

    // Create the graphics pipeline.
    // This function is designed to take in multiple pipelineCreateInfos
    // and create multiple pipelines with them. 
    // This function alos allows us to pass in a VkPipelineCache, which is
    // used to store and reuse data relevant to pipeline creation. 
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, 
            nullptr, &graphicsPipeline) 
            != VK_SUCCESS) {
        PONG_FATAL_ERROR("Failed to create graphics pipeline!");
    }

    // Delete the shader modules (doesn't need to happen during device cleanup
    // phase)
    vkDestroyShaderModule(device, vertShaderModule, nullptr); 
    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    
    // Delete the vertex and frag shaders (at least for now)
    // TODO: move this into it's own method
    delete [] vert.p_byteCode;
    delete [] frag.p_byteCode;

    // --------------------- FRAMEBUFFER SETUP --------------------------
    
    // At this point, we now need to create the framebuffers which will be 
    // used for our renderpass. These framebuffers should be used in the 
    // format as our swapchain images (which we defined before).

    // We use a VkFramebuffer object to store the attachments which were specified
    // during the render pass. A framebuffer references all the image views that
    // represent these attachments. In our case, we only have one to reference, 
    // namely the color attachment. 

    // First, we create an array to hold our framebuffers. 
    VkFramebuffer swapChainFramebuffers[imageCount];

    for (size_t i = 0; i < imageCount; i++) {
        
        // Get the image stored previously by our swapchain
        VkImageView attachments[] = {
            swapChainImageViews[i]
        };
        
        // Create a new framebuffer which uses our renderpass and image.
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        // specify the renderpass - can only be done with a compatible render pass. 
        // This means that the number of attachments in the renderpass must be 
        // the same as the number of attachments in the framebuffer. 
        framebufferInfo.renderPass = renderPass;
        // Now we specify the image views associated with this framebuffer.
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = chosenExtent.width;
        framebufferInfo.height = chosenExtent.height;
        // Refers the the number of layers in image arrays. 
        framebufferInfo.layers = 1;
        
        // Create the framebuffer itself
        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) 
                != VK_SUCCESS) {

            PONG_FATAL_ERROR("Failed to create framebuffer!");
        }
    }
 
    // ======================= END OF SETUP =============================

    // ------------------------- MAIN LOOP ------------------------------

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    // --------------------------- CLEANUP ------------------------------
    for (size_t i = 0; i < imageCount; i++) {
        vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
    }

    // Destroy the graphics pipeline  
    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    // Clean up pipeline memory
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    // Destroy the render pass
    vkDestroyRenderPass(device, renderPass, nullptr);
    // Cleaning up memory
    if(enableValidationLayers) {
        destroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }

    // Destroy image views
    for (size_t i = 0; i < imageCount; i++) {
        vkDestroyImageView(device, swapChainImageViews[i], nullptr);
    }

    // Destroy the Swapchain
    vkDestroySwapchainKHR(device, swapchain, nullptr);

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
