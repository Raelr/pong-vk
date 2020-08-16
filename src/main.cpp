#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <cstring>
#include <vector>
#include "logger.h"
#include <string>
#include <set>
#include <cstdint>
#include "vulkanUtils.h"
#include "renderer/renderer.h"

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

//// A debug function callback - where the message data actually goes when triggered by the
//// validation layer.
//// This function uses three mactros to define it's signature (used to help Vulkan know that
//// this method is valid).
//static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
//    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,     // The severity of the message
//    VkDebugUtilsMessageTypeFlagsEXT messageType,                // The actual TYPE of message
//    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,  // Struct with data of the message
//    void* pUserData) {                                          // User defined data (optional)
//
//    // In this case we simply print out the message itself to the console.
//    ERROR(pCallbackData->pMessage);
//
//    // Return value determines whether the call that triggers this message should be aborted.
//    // Generally these should return false.
//    return VK_FALSE;
//}
//
//// A proxy function which calls the vkCreateDebugUtilsMessengerEXT function.
//// The reason we need this function is because 'vkCreateDebugUtilsMessengerEXT' is an
//// extension function - meaning that it isn't automatically loaded into memory. As such,
//// we need to manually look up it's memory and call it from there. Vulkan provides us
//// with a utility function: 'vkGetInstanceProcAddr' for this exact purpose.
//static VkResult createDebugUtilsMessengerEXT(VkInstance instance,
//    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator,
//    VkDebugUtilsMessengerEXT* pDebugMessenger) {
//
//    // Store the function in a variable 'func'.
//    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
//        instance, "vkCreateDebugUtilsMessengerEXT");
//
//    // Make sure we got the correct function.
//    if (func != nullptr) {
//        // Call the function
//        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
//    } else {
//        // Return an error
//        return VkResult::VK_ERROR_EXTENSION_NOT_PRESENT;
//    }
//}
//
//// Simply accepts a reference to a messenger config struct and fills it with data.
//void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& messengerInfo) {
//    messengerInfo = {};
//    // Specify the type of struct that's being populated
//    messengerInfo.sType =           VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
//    // You can specify the severity levels of logs that this struct will accept.
//    messengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
//                                    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
//                                    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
//    // You can also limit the actual message type to specific types.
//    messengerInfo.messageType =     VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
//                                    | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
//                                    | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
//    // Finally, you can store the actual debug callback in the struct
//    messengerInfo.pfnUserCallback = debugCallback;
//    messengerInfo.pUserData = nullptr;
//}

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
    
    while (*width == 0 && *height == 0) {
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

void updateUniformBuffer(VkDeviceMemory* memory, VkDevice device, 
    VkExtent2D swapchainExtent, float x, float y) {

    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();

    float time = std::chrono::duration<float, 
        std::chrono::seconds::period>(currentTime - startTime).count();

    Buffers::UniformBufferObject ubo{};

    glm::mat4 m = glm::mat4(1.0f);
    
    m = glm::translate(m, glm::vec3(x, y, 0.0f));
    
    m = glm::rotate(m, time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    m = glm::scale(m, glm::vec3(200.0f, 200.0f, 1.0f));

    // Rotate the model matrix
    ubo.model = m;

    // Set the view
    ubo.view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -1.0f));
    // Set our projection to be an orthographic view (for 3D)
    ubo.proj = glm::ortho(
        -(static_cast<float>(WINDOW_WIDTH) / 2), 
        static_cast<float>(WINDOW_WIDTH) / 2, 
        static_cast<float>(WINDOW_HEIGHT) / 2, 
        -(static_cast<float>(WINDOW_HEIGHT) / 2), -1.0f, 1.0f);

    // Now we bind our data to the UBO for later use
    void* data;
    vkMapMemory(device, *memory, 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(device, *memory);
}

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

    // ========================= RENDERER ================================

    // ==================== INITIALISE RENDERER ==========================

    Renderer::Renderer renderer;

    // Let the renderer know that we want to load in default validation layers
    Renderer::loadDefaultValidationLayers(&renderer);
    Renderer::loadDefaultDeviceExtensions(&renderer);

    if (Renderer::initialiseRenderer(&renderer, enableValidationLayers, window)
        != Renderer::Status::SUCCESS) {
        PONG_FATAL_ERROR("Failed to initialise renderer!");
    }
    
    // ------------------------ RENDER PASS -----------------------------

    VulkanUtils::GraphicsPipelineData graphicsPipeline;
    
    // In Vulkan, a render pass represents all information that our 
    // framebuffers will need to process our images.
    if (VulkanUtils::createRenderPass(renderer.deviceData.logicalDevice,
        renderer.swapchainData.swapchainFormat, &graphicsPipeline) != VK_SUCCESS) {

        PONG_FATAL_ERROR("Failed to create render pass!");
    }

    // ------------------- DESCRIPTOR SET LAYOUT -----------------------------

    VkDescriptorSetLayout descriptorSetLayout{};

    if (VulkanUtils::createDescriptorSetLayout(renderer.deviceData.logicalDevice,
        &descriptorSetLayout) != VK_SUCCESS) {

        PONG_FATAL_ERROR("Failed to create descriptor set!");

    }

    // --------------------- GRAPHICS PIPELINE -------------------------------
    
    // Vulkan requires that you define your own graphics pipelines when you
    // want to use different combinations of shaders. This is because the 
    // graphics pipeline in Vulkan is almost completely immutable. This means
    // that the pipeline can be very well optimised (but will also require
    // a complete rewrite if you need anything different).
    
    if (VulkanUtils::createGraphicsPipeline(renderer.deviceData.logicalDevice,
        &graphicsPipeline, &renderer.swapchainData, &descriptorSetLayout) != VK_SUCCESS) {
        
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
    std::vector<VkFramebuffer> swapchainFramebuffers(renderer.swapchainData.imageCount);

    if (VulkanUtils::createFramebuffer(renderer.deviceData.logicalDevice,
        swapchainFramebuffers.data(), &renderer.swapchainData, &graphicsPipeline)
        != VK_SUCCESS) {
        
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
    poolInfo.queueFamilyIndex = renderer.deviceData.indices.graphicsFamily.value();
    poolInfo.flags = 0; // optional
    
    // Create the command pool
    if (vkCreateCommandPool(renderer.deviceData.logicalDevice, &poolInfo, nullptr,
        &commandPool) != VK_SUCCESS) {
        
        PONG_FATAL_ERROR("Failed to create command pool!");
    }

    // --------------------- VERTEX DEFINITION --------------------------

    Buffers::VertexBuffer vertexBuffer;
    Buffers::IndexBuffer indexBuffer;
    // Define a vertex buffer for configuring vertex data.
    vertexBuffer.vertexCount = 4;

    {
        // Define all the vertices of our triangle and the colors for every vertex
        Buffers::Vertex vertices[] = {
            // Positions        // Colors
            { {-0.5f, -0.5f},   {1.0f, 1.0f, 1.0f} },
            { {0.5f, -0.5f},    {1.0f, 1.0f, 1.0f} },
            { {0.5f, 0.5f},     {1.0f, 1.0f, 1.0f} },
            { {-0.5f, 0.5f},    {1.0f, 1.0f, 1.0f} }
        };

        vertexBuffer.vertices = vertices;
    }

    uint16_t indices[] = {
        0, 1, 2, 2, 3, 0
    };

    indexBuffer.indexCount = 6;
    indexBuffer.indices = indices;

    // Create the vertex buffer and allocate the memory for it
    if (VulkanUtils::createVertexBuffer(&renderer.deviceData, &vertexBuffer, commandPool)
        != VK_SUCCESS) {
    
        PONG_FATAL_ERROR("Failed to create vertex buffer.");
    }

    // Create a uniform buffer for storing vertex data. 
    if (VulkanUtils::createIndexBuffer(&renderer.deviceData, &indexBuffer, commandPool)
        != VK_SUCCESS) {

        PONG_FATAL_ERROR("Failed to create index buffer.");
    }

    size_t objects = 2;
    // Define a vector for storing uniform buffer information
    Buffers::BufferData uniformBuffers[renderer.swapchainData.imageCount];
    Buffers::BufferData uniformBuffers2[renderer.swapchainData.imageCount];

    // Create our uniform buffers (one per image)
    if (VulkanUtils::createUniformBuffers(&renderer.deviceData, uniformBuffers,
        renderer.swapchainData.imageCount) != VK_SUCCESS) {

        PONG_FATAL_ERROR("Failed to create uniform buffers!");
    }

    // Create uniform buffers for second object
    if (VulkanUtils::createUniformBuffers(&renderer.deviceData, uniformBuffers2,
        renderer.swapchainData.imageCount) != VK_SUCCESS) {

        PONG_FATAL_ERROR("Failed to create uniform buffers!");
    }
    
    Buffers::BufferData* uBuffers[] = {
        uniformBuffers, uniformBuffers2
    };
    // ---------------------- DESCRIPTOR POOL --------------------------------

    VkDescriptorPool descriptorPool{};

    if (VulkanUtils::createDescriptorPool(renderer.deviceData.logicalDevice,
        renderer.swapchainData.imageCount, &descriptorPool, objects) != VK_SUCCESS) {

        PONG_FATAL_ERROR("Failed to create descriptor pool.");
    }

    // --------------------------- DESCRIPTOR SETS ----------------------------

    VkDescriptorSet descriptorSets[renderer.swapchainData.imageCount];
    VkDescriptorSet descriptorSets2[renderer.swapchainData.imageCount];

    if (VulkanUtils::createDescriptorSets(&renderer.deviceData, descriptorSets,
        &descriptorSetLayout, &descriptorPool, renderer.swapchainData.imageCount,
        uBuffers[0]) != VK_SUCCESS) {
        
        PONG_FATAL_ERROR("Failed to create descriptor sets!");
    }

    if (VulkanUtils::createDescriptorSets(&renderer.deviceData, descriptorSets2,
        &descriptorSetLayout, &descriptorPool, renderer.swapchainData.imageCount,
        uBuffers[1]) != VK_SUCCESS) {
        
        PONG_FATAL_ERROR("Failed to create descriptor sets!");
    }

    // ------------------ COMMAND BUFFER CREATION -----------------------

    // With the command pool created, we can now start creating and allocating
    // command buffers. Because these commands involve allocating a framebuffer,
    // we'll need to record a command buffer for every image in the swap chain.
    
    VkDescriptorSet* sets[] =  {
        descriptorSets, descriptorSets2
    };

    // Make this the same size as our images. 
    std::vector<VkCommandBuffer> commandBuffers(renderer.swapchainData.imageCount);

    // It should be noted that command buffers are automatically cleaned up when
    // the commandpool is destroyed. As such they require no explicit cleanup.

    if (VulkanUtils::createCommandBuffers(renderer.deviceData.logicalDevice,
        commandBuffers.data(), &graphicsPipeline, &renderer.swapchainData,
        swapchainFramebuffers.data(), commandPool, &vertexBuffer, &indexBuffer,
        sets, objects)
        != VK_SUCCESS) {

        PONG_FATAL_ERROR("Failed to create command buffers!");
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
    std::vector<VkFence> imagesInFlight(renderer.swapchainData.imageCount);

    // Initialise all these images to 0 to start with. 
    for (size_t i = 0; i < renderer.swapchainData.imageCount; i++) {
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
        if (vkCreateSemaphore(renderer.deviceData.logicalDevice, &semaphoreInfo, nullptr,
            &imageAvailableSemaphores[i]) != VK_SUCCESS 
            || 
            vkCreateSemaphore(renderer.deviceData.logicalDevice, &semaphoreInfo, nullptr,
            &renderFinishedSemaphores[i]) != VK_SUCCESS 
            ||
            vkCreateFence(renderer.deviceData.logicalDevice, &fenceInfo, nullptr,
            &inFlightFences[i]) != VK_SUCCESS) {

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
        vkWaitForFences(renderer.deviceData.logicalDevice, 1,
            &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

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
        VkResult result = vkAcquireNextImageKHR(renderer.deviceData.logicalDevice,
            renderer.swapchainData.swapchain, UINT64_MAX, imageAvailableSemaphores[currentFrame],
            VK_NULL_HANDLE, &imageIndex);
        // If our swapchain is out of date (no longer valid, then we re-create
        // it)
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {

            handleMinimisation(window, &renderer.deviceData.framebufferWidth,
                &renderer.deviceData.framebufferHeight);

            // Re-create the swap chain in its entirety if the pipeline is no 
            // longer valid or is out of date. 
            VulkanUtils::recreateSwapchain(
                &renderer.deviceData,
                &renderer.swapchainData,
                &graphicsPipeline, 
                commandPool, 
                swapchainFramebuffers.data(), 
                &vertexBuffer,
                &indexBuffer,
                commandBuffers.data(),
                &descriptorSetLayout,
                &descriptorPool,
                sets,
                uBuffers,
                objects
            );
            
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
            vkWaitForFences(renderer.deviceData.logicalDevice, 1, &imagesInFlight[imageIndex],
                VK_TRUE, UINT64_MAX);
        }
        // Now, use the image in this frame!.
        imagesInFlight[imageIndex] = inFlightFences[currentFrame];

        updateUniformBuffer(&uBuffers[0][imageIndex].bufferMemory, 
            renderer.deviceData.logicalDevice, renderer.swapchainData.swapchainExtent, -200.0f, 0.0f);

        updateUniformBuffer(&uBuffers[1][imageIndex].bufferMemory, 
            renderer.deviceData.logicalDevice, renderer.swapchainData.swapchainExtent, 200.0f, 0.0f);
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
        vkResetFences(renderer.deviceData.logicalDevice, 1, &inFlightFences[currentFrame]);
       
        // Finally, we submit the buffer to the graphics queue 
        if (vkQueueSubmit(renderer.deviceData.graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame])
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
        VkSwapchainKHR swapchains[] = { renderer.swapchainData.swapchain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapchains;
        // Get the index of the image we're using:
        presentInfo.pImageIndices = &imageIndex;
        // Allows you to get an array of VK_RESULTs which tell you if presentation
        // of every swapchain was successful. 
        presentInfo.pResults = nullptr; // optional
        
        // Now we submit a request to present an image to the swapchain. 
        result = vkQueuePresentKHR(renderer.deviceData.presentQueue, &presentInfo);
        // Again, we make sure that we're using the best possible swapchain.
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR 
            || pongData.framebufferResized) {
            
            handleMinimisation(window,&renderer.deviceData.framebufferWidth,
                &renderer.deviceData.framebufferHeight);
            
            // Re-create the swap chain in its entirety if the pipeline is no
            // longer valid or is out of date.
            VulkanUtils::recreateSwapchain(
                &renderer.deviceData,
                &renderer.swapchainData,
                &graphicsPipeline,
                commandPool,
                swapchainFramebuffers.data(),
                &vertexBuffer,
                &indexBuffer,
                commandBuffers.data(),
                &descriptorSetLayout,
                &descriptorPool,
                sets,
                uBuffers,
                objects);

            pongData.framebufferResized = false;
        } else if (result != VK_SUCCESS) {

            PONG_FATAL_ERROR("Failed to present swapchain image!");

        }
        
        // This should clamp the value of currentFrame between 0 and 1.
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    // Since our image drawing is asynchronous, we need to make sure that
    // our resources aren't in use when trying to clean them up:
    vkDeviceWaitIdle(renderer.deviceData.logicalDevice);
    
    // --------------------------- CLEANUP ------------------------------

    VulkanUtils::cleanupSwapchain(
        renderer.deviceData.logicalDevice,
        &renderer.swapchainData,
        &graphicsPipeline, 
        commandPool, 
        swapchainFramebuffers.data(), 
        commandBuffers.data(),
        uBuffers,
        descriptorPool,
        objects
    );

    vkDestroyDescriptorSetLayout(renderer.deviceData.logicalDevice, descriptorSetLayout,
        nullptr);

    // Cleans up the memory buffer 
    vkDestroyBuffer(renderer.deviceData.logicalDevice, vertexBuffer.bufferData.buffer, nullptr);

    // Frees the allocated vertex buffer memory 
    vkFreeMemory(renderer.deviceData.logicalDevice, vertexBuffer.bufferData.bufferMemory, nullptr);

    vkDestroyBuffer(renderer.deviceData.logicalDevice, indexBuffer.bufferData.buffer, nullptr);

    vkFreeMemory(renderer.deviceData.logicalDevice, indexBuffer.bufferData.bufferMemory, nullptr);

    // Clean up the semaphores we created earlier.
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(renderer.deviceData.logicalDevice, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(renderer.deviceData.logicalDevice, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(renderer.deviceData.logicalDevice, inFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(renderer.deviceData.logicalDevice, commandPool, nullptr);

    // Cleaning up memory
    if(enableValidationLayers) {
        destroyDebugUtilsMessengerEXT(renderer.instance, renderer.debugMessenger, nullptr);
    }

    // Destroy window surface
    vkDestroySurfaceKHR(renderer.instance, renderer.deviceData.surface, nullptr);

    // Destroy logical device
    vkDestroyDevice(renderer.deviceData.logicalDevice, nullptr);

    // Vulkan cleanup
    vkDestroyInstance(renderer.instance, nullptr);

    free(renderer.extensions);
    
    // GLFW cleanup
    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS; 
} 
