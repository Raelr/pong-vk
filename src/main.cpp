#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include "logger.h"
#include <string>
#include <set>
#include <cstdint>
#include "utils.h"
#include "vulkanUtils.h"

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

// Specifying how many frames we allow to re-use for the next frame.
const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

// Only enable validation layers when the program is run in DEBUG mode.
#ifdef DEBUG
    const bool enableValidationLayers = true;
#else
    const bool enableValidationLayers = false;
#endif

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

// Handles a case where the window is minimised
void handleMinimisation(GLFWwindow* window, int* width, int* height) {
    
    glfwGetFramebufferSize(window, width, height);
    
    // If the window is minimized we simply pause rendering until it
    // comes back!
    
    while (width == 0 && height == 0) {
        glfwGetFramebufferSize(window, width, height);
        glfwWaitEvents();
    }
}

// Struct for storing application-specific data. Will be used by glfw for data
// storage in the user pointer.
struct AppData {
    // Boolean for checking if the window has been resized
    bool framebufferResized;
};

int main() {  

    // -------------------- INITIALISE WINDOW --------------------------
    initLogger();
    
    // Initialise app data struct
    AppData pongData{};
    pongData.framebufferResized = false;

    // Initialise GLFW
    glfwInit();

    // Set flags for the window (set it to not use GLFW API and 
    // set it to not resize)
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // Actually make the window
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Pong", 
        nullptr, nullptr);

    glfwSetWindowUserPointer(window, &pongData);

    // NOTE: THIS IS JUST A TEST, WILL NEED TO SET A BETTER POINTER THAN A 
    // SINGLE INT IN FUTURE
    // TODO: Replace this resize boolean with a struct containing our app data.
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, 
        int height) {

        AppData* data = 
            reinterpret_cast<AppData*>(glfwGetWindowUserPointer(window));

        data->framebufferResized = true;

    });

    INFO("Created GLFW window");

    // ========================= VULKAN SETUP ================================

    // ------------------ VALIDATION LAYER SUPPORT CHECKING ------------------

    uint32_t layerCount = 0;

    // Get the number of layers available for vulkan.
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> layerProperties(layerCount);

    if (enableValidationLayers) {
        
        // Get the layer names themselves USING the count variable.
        vkEnumerateInstanceLayerProperties(&layerCount, layerProperties.data());

        uint8_t foundCount = 0;

        // Search to see whether the validation layers requested are supported by Vulkan.
        for (size_t i = 0; i < validationLayersCount; i++) {
            for (auto& layer : layerProperties) {
                if (strcmp(validationLayers[i], layer.layerName) == 0) {
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

    std::vector<VkExtensionProperties> vkExtensions(vulkanExtensionCount);

    // This instance of the method will return the exact extensions supported
    vkEnumerateInstanceExtensionProperties(nullptr, &vulkanExtensionCount, vkExtensions.data());

    // Print out and display all extensions.
    INFO("Checking Extensions: ");

    std::string extensionStr = "\n";
    for (auto& extension : vkExtensions) {
        std::string str = extension.extensionName;
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
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    // Now we need to actually find a device to use. This can be done using a number of
    // methods. One such method is to assign a rating to each GPU and pick the best rated one. 
    // In this case, since this is a tutorial, we'll just take the first GPU we find. 
    for (auto& device: devices) {

        // Now we need to actually find the exact queue families that this device uses. 
        // In Vulkan, all commands are executed in a queue of similar types. 
        // We need to find exactly which queues this device uses so we can use them
        // to execute commands. 
        
        VulkanUtils::QueueFamilyIndices indices = 
                VulkanUtils::findQueueFamilies(device, surface);

        // Now we need to check whether our physical device supports drawing images to the screen.

        // Get the number of extensions
        uint32_t extensionCount = 0;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        // Store the unique extensions in a set so we can quickly remove duplicates.
        std::set<std::string> requiredExtensions(deviceExtensions+0, deviceExtensions+deviceExtensionsCount);

        for (auto& extension : availableExtensions) {
            // If all required extensions are available from our GPU then we know we have full 
            // Vulkan support!
            requiredExtensions.erase(extension.extensionName);
        }

        bool extensionsSupported = requiredExtensions.empty();
        
        // Check if we have support for swapchains!
        bool swapChainAdequate = false;

        // Only check if we have supported extensions.
        if (extensionsSupported) {
            // Get the swapchain details
            VulkanUtils::SwapchainSupportDetails supportDetails = 
                    VulkanUtils::querySwapchainSupport(device, surface);
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
    VulkanUtils::QueueFamilyIndices indices = 
            VulkanUtils::findQueueFamilies(physicalDevice, surface);

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

    // Create a basic swapchain strucutre - this is a custom struct that
    // stores all our swapchain data.
    VulkanUtils::SwapchainData swapchain;

    int width, height;

    glfwGetFramebufferSize(window, &width, &height);

    // Create the swapchain (should initialise both the swapchain and image
    // views). See vulkanUtils.cpp for implementation details.
    if (VulkanUtils::createSwapchain(&swapchain, physicalDevice, device,
            surface, static_cast<uint32_t>(width), static_cast<uint32_t>(height), 
            indices) != VK_SUCCESS) {

       PONG_FATAL_ERROR("Failed to create swapchain!");
    }
    
    // ------------------------ RENDER PASS -----------------------------
   
    VulkanUtils::GraphicsPipelineData graphicsPipeline;
    
    // In Vulkan, a render pass represents all information that our 
    // framebuffers will need to process our images.
    if (VulkanUtils::createRenderPass(device, swapchain.swapchainFormat, 
            &graphicsPipeline) != VK_SUCCESS) {
        PONG_FATAL_ERROR("Failed to create render pass!");
    }

    // --------------------- GRAPHICS PIPELINE --------------------------
    
    // Vulkan requires that you define your own graphics pipelines when you
    // want to use different combinations of shaders. This is because the 
    // graphics pipeline in Vulkan is almost completely immutable. This means
    // that the pipeline can be very well optimised (but will also require
    // a complete rewrite if you need anything different).
    
    if (VulkanUtils::createGraphicsPipeline(device, &graphicsPipeline, 
            &swapchain) != VK_SUCCESS) {
        PONG_FATAL_ERROR("Failed to create graphics pipeline!");

    }

    // --------------------- FRAMEBUFFER SETUP --------------------------
    
    // At this point, we now need to create the framebuffers which will be 
    // used for our renderpass. These framebuffers should be used in the 
    // format as our swapchain images (which we defined before).

    // We use a VkFramebuffer object to store the attachments which were specified
    // during the render pass. A framebuffer references all the image views that
    // represent these attachments. In our case, we only have one to reference, 
    // namely the color attachment.
    
    // First, we create an array to hold our framebuffers. 
    std::vector<VkFramebuffer> swapchainFramebuffers(swapchain.imageCount);

    if (VulkanUtils::createFramebuffer(device, swapchainFramebuffers.data(),
            &swapchain, &graphicsPipeline) != VK_SUCCESS) {
        PONG_FATAL_ERROR("Failed to create framebuffers!");
    }

    // ------------------- COMMAND POOL CREATION ------------------------

    // In vulkan, all steps and operations that happen are not handled via
    // function calls. Rather, these steps are recorded in a CommandBuffer 
    // object which are then executed later in runtime. As such, commands
    // allow us to set everything up in advance and in multiple threds if 
    // need be. These commands are then handled by Vulkan in the main loop. 
    
    // First, we need to create a command pool, which manage the memory used 
    // to allocate and store the command buffers which are given to them. We do
    // this with the VkCommandPool struct:
    VkCommandPool commandPool;

    // Command buffers are generally executed by submitting them to one of the 
    // device queues (such as the graphics and presentation queues we set earlier).
    // Command pools can only submit buffers to a single type of queue. Since 
    // we're going to be submitting data for drawing, we'll use the graphics 
    // queue. 

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = indices.graphicsFamily.value();
    poolInfo.flags = 0; // optional
    
    // Create the command pool
    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) 
            != VK_SUCCESS) {
        PONG_FATAL_ERROR("Failed to create command pool!");
    }

    // ------------------ COMMAND BUFFER CREATION -----------------------

    // With the command pool created, we can now start creating and allocating
    // command buffers. Because these commands involve allocating a framebuffer,
    // we'll need to record a command buffer for every image in the swap chain.
    
    // Make this the same size as our images. 
    std::vector<VkCommandBuffer> commandBuffers(swapchain.imageCount);

    // It should be noted that command buffers are automatically cleaned up when
    // the commandpool is destroyed. As such they require no explicit cleanup.

    if (VulkanUtils::createCommandBuffers(device, commandBuffers.data(), 
            &graphicsPipeline, &swapchain, swapchainFramebuffers.data(), 
            commandPool) != VK_SUCCESS) {

    }
    // --------------------- SYNC OBJECT CREATION -------------------------
    
    // In Vulkan, drawing every frame can be done in a few simple function 
    // calls. These function calls are executed asynchronously - they are 
    // executed and returned in no particular order. As such, we aren't always
    // guaranteed to get the images we need to execute the next frame. This 
    // would then lead to serious memory violations. 

    // As such, Vulkan requires us to synchronise our swap chain events, namely
    // through the use of semaphores and fences. In both cases, our program will
    // be forced to stop execution until a certain resource is made available
    // to it (when the semaphore moves from 'signalled' to 'unsignalled'). 
    
    // The main difference between semaphores and fences is that fences can be 
    // accessed by our program through a number of function calls. Semaphores, 
    // on the other hand, cannot. Semaphores are primarily used to synchornise 
    // operations within or across command queues. 
    
    // For now, we'll be using sempahores to get a triangle rendering!

    // Create two semaphores - one which signals when an image is ready to be
    // used, another for when the render pass has finished. 
    VkSemaphore imageAvailableSemaphores [MAX_FRAMES_IN_FLIGHT];
    VkSemaphore renderFinishedSemaphores [MAX_FRAMES_IN_FLIGHT];
    
    // The semaphore info struct only has one field
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    // We'll also create a fence to keep our CPU and GPU fully synced with one
    // another. Fences are similar to semaphores, with their main difference 
    // being that you must wait for them in code. 
    VkFence inFlightFences[MAX_FRAMES_IN_FLIGHT];
    // We'll have a second array of fences to track whether an image is being
    // used by a fence. 
    VkFence imagesInFlight[swapchain.imageCount];

    // Initialise all these images to 0 to start with. 
    for (size_t i = 0; i < swapchain.imageCount; i++) {
        imagesInFlight[i] = VK_NULL_HANDLE;
    }
    // As always with Vulkan, we create a create info struct to handle the 
    // configuration.
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    // Specify that the fence should be started in a signalled state.
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    // Now we simply create both semaphores, making sure that both succeed before we 
    // move on.
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) 
            != VK_SUCCESS 
            || 
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i])
            != VK_SUCCESS 
            ||
            vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {

            PONG_FATAL_ERROR("Failed to create synchronisation objects for frame!");
        }
    }

    // ======================= END OF SETUP =============================

    // ------------------------- MAIN LOOP ------------------------------

    size_t currentFrame = 0;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // This function takes an array of fences and waits for either one or all
        // of them to be signalled. The fourth parameter specifies that we're 
        // waiting for all fences to be signalled before moving on. The last 
        // parameter takes a timeout period which we set really high (effectively
        // making it null)
        vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        // In each frame of the main loop, we'll need to perform the following
        // operations:
        // 1. acquire an image from the swapchain.
        // 2. execute the command buffer with that image as an attachment.
        // 3. Return the image to the swapchain for presentation.

        uint32_t imageIndex;
        // First we need to acquire an image from the swapchain. For this we need
        // our logical device and swapchain. The third parameter is a timeout period
        // which we disable using the max of a 64-bit integer. Next we provide our
        // semaphore, and finally a variable to output the image index to. 
        VkResult result = vkAcquireNextImageKHR(device, swapchain.swapchain, 
            UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, 
            &imageIndex);
        // If our swapchain is out of date (no longer valid, then we re-create
        // it)
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
           
            // TODO: craete a standalone function for handling re-creation of 
            // swapchains for re-usability.
            // TODO: Move all swapchain items into a standalone struct for 
            // storage
            handleMinimisation(window, &width, &height);
            // Re-create the swap chain in its entirety if the pipeline is no 
            // longer valid or is out of date. 
            VulkanUtils::recreateSwapchain(
                device, 
                physicalDevice, 
                &swapchain,
                surface, 
                static_cast<uint32_t>(width), 
                static_cast<uint32_t>(height), 
                indices, 
                &graphicsPipeline, 
                commandPool, 
                swapchainFramebuffers.data(), 
                commandBuffers.data());
                
                pongData.framebufferResized = false;
            // Go to the next iteration of the loop
            continue;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            PONG_FATAL_ERROR("Failed to acquire swapchain image!");
        }
        
        // Check if a previous frame is using this image. I.e: we're waiting on 
        // it's fence to be signalled. 
        if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
            // Wait for the fence to signal that it's available for usage. This 
            // will now ensure that there are no more than 2 frames in use, and 
            // that these frames are not accidentally using the same image!
            vkWaitForFences(device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
        }
        // Now, use the image in this frame!.
        imagesInFlight[imageIndex] = inFlightFences[currentFrame];

        // Once we have that, we now need to submit the image to the queue:
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        
        // We need to specify which semaphores to wait on before executing the frame.
        VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
        // We also need to specify which stages of the pipeline need to be done so we can move
        // on.
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        // A count of all semaphores.
        submitInfo.waitSemaphoreCount = 1;
        // Finaly, input the semaphores and stages. 
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages; 
        // Now we need to specify which command buffers to submit to. In our
        // case we need to submit to the buffer which corresponds to our image.
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
        // Now we specify which semaphores we need to signal once our command buffers
        // have finished execution. 
        VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        // We need to reset the fence to an unsignalled state before moving on.
        vkResetFences(device, 1, &inFlightFences[currentFrame]);
       
        // Finally, we submit the buffer to the graphics queue 
        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) 
            != VK_SUCCESS) {
            PONG_FATAL_ERROR("Failed to submit draw command buffer!");
        }
        
        // The final step to drawing a frame is resubmitting the the result back
        // to the swapchain. This is done by configuring our swapchain presentation.

        // Start with a PresentInfo struct:
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        // First we specify how many semaphores we need to wait on.
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        // Next we need to specify which swapchains we're using:
        VkSwapchainKHR swapchains[] = { swapchain.swapchain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapchains;
        // Get the index of the image we're using:
        presentInfo.pImageIndices = &imageIndex;
        // Allows you to get an array of VK_RESULTs which tell you if presentation
        // of every swapchain was successful. 
        presentInfo.pResults = nullptr; // optional
        
        // Now we submit a request to present an image to the swapchain. 
        result = vkQueuePresentKHR(presentQueue, &presentInfo);
        // Again, we make sure that we're using the best possible swapchain.
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR 
            || pongData.framebufferResized) {
            
            handleMinimisation(window,& width, &height);
            
            // Re-create the swap chain in its entirety if the pipeline is no
            // longer valid or is out of date.
            VulkanUtils::recreateSwapchain(
                device,
                physicalDevice,
                &swapchain,
                surface,
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height),
                indices,
                &graphicsPipeline,
                commandPool,
                swapchainFramebuffers.data(),
                commandBuffers.data());
                pongData.framebufferResized = false;
        } else if (result != VK_SUCCESS) {

            PONG_FATAL_ERROR("Failed to present swapchain image!");

        }
        
        // This should clamp the value of currentFrame between 0 and 1.
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    // Since our image drawing is asynchronous, we need to make sure that
    // our resources aren't in use when trying to clean them up:
    vkDeviceWaitIdle(device);
    
    // --------------------------- CLEANUP ------------------------------
    
    VulkanUtils::cleanupSwapchain(device, &swapchain, &graphicsPipeline, 
        commandPool, swapchainFramebuffers.data(), commandBuffers.data());

    // Clean up the semaphores we created earlier.
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(device, commandPool, nullptr);

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
