#define GLFW_INCLUDE_VULKAN
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#include <GLFW/glfw3.h>
#include <cstdlib>
#include "logger.h"
#include <set>
#include <cstdint>
#include "vulkanUtils.h"
#include "renderer/renderer.h"

#define PONG_FATAL_ERROR(...) ERROR(__VA_ARGS__); return EXIT_FAILURE

// Const variables to determine initial window height ad 
const int WINDOW_HEIGHT = 600;
const int WINDOW_WIDTH = 800;

// Specifying how many frames we allow to re-use for the next frame.
const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

// Only enable validation layers when the program is run in DEBUG mode.
#ifdef DEBUG
    const bool enableValidationLayers = true;
#else
    const bool enableValidationLayers = false;
#endif

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

void updateUniformBuffer(VkDeviceMemory* memory, VkDevice device, float x, float y) {

    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();

    float time = std::chrono::duration<float, 
        std::chrono::seconds::period>(currentTime - startTime).count();

    Buffers::UniformBufferObject ubo{};
    
    ubo.mvp = glm::translate(ubo.mvp, glm::vec3(x, y, 0.0f));
    
    ubo.mvp = glm::rotate(ubo.mvp, time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    ubo.mvp = glm::scale(ubo.mvp, glm::vec3(200.0f, 200.0f, 1.0f));

    // Set the view
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -1.0f));
    // Set our projection to be an orthographic view (for 3D)
    glm::mat4 proj = glm::ortho(
            -(static_cast<float>(WINDOW_WIDTH) / 2),
            static_cast<float>(WINDOW_WIDTH) / 2,
            static_cast<float>(WINDOW_HEIGHT) / 2,
            -(static_cast<float>(WINDOW_HEIGHT) / 2), -1.0f, 1.0f);

    // Rotate the model matrix
    ubo.mvp = proj * view * ubo.mvp;

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

    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, 
        int height) {

        auto data =
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
        &renderer.renderer2DData.quadData.descriptorSetLayout,
        &descriptorPool, renderer.swapchainData.imageCount,uBuffers[0]) != VK_SUCCESS) {
        
        PONG_FATAL_ERROR("Failed to create descriptor sets!");
    }

    if (VulkanUtils::createDescriptorSets(&renderer.deviceData, descriptorSets2,
        &renderer.renderer2DData.quadData.descriptorSetLayout, &descriptorPool,
        renderer.swapchainData.imageCount,uBuffers[1]) != VK_SUCCESS) {
        
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
    VkCommandBuffer commandBuffers[renderer.swapchainData.imageCount];

    // It should be noted that command buffers are automatically cleaned up when
    // the commandpool is destroyed. As such they require no explicit cleanup.

    if (VulkanUtils::createCommandBuffers(renderer.deviceData.logicalDevice,
          commandBuffers, &renderer.renderer2DData.graphicsPipeline, &renderer.swapchainData,
          renderer.renderer2DData.frameBuffers, renderer.renderer2DData.commandPool,
          &renderer.renderer2DData.quadData.vertexBuffer, &renderer.renderer2DData.quadData.indexBuffer,
          sets, objects)
        != VK_SUCCESS) {

        PONG_FATAL_ERROR("Failed to create command buffers!");
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
            &renderer.inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

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
            renderer.swapchainData.swapchain, UINT64_MAX, renderer.imageAvailableSemaphores[currentFrame],
            VK_NULL_HANDLE, &imageIndex);

        if (vkGetFenceStatus(renderer.deviceData.logicalDevice, renderer.inFlightFences[currentFrame])
            == VK_SUCCESS) {

            vkResetCommandBuffer(commandBuffers[imageIndex], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

            if (VulkanUtils::createCommandBuffers(renderer.deviceData.logicalDevice,
              commandBuffers, &renderer.renderer2DData.graphicsPipeline, &renderer.swapchainData,
              renderer.renderer2DData.frameBuffers, renderer.renderer2DData.commandPool,
              &renderer.renderer2DData.quadData.vertexBuffer, &renderer.renderer2DData.quadData.indexBuffer,
              sets, objects)
                != VK_SUCCESS) {

                PONG_FATAL_ERROR("Failed to create command buffers!");
            }
        }

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
                &renderer.renderer2DData.graphicsPipeline,
                renderer.renderer2DData.commandPool,
                renderer.renderer2DData.frameBuffers,
                &renderer.renderer2DData.quadData.vertexBuffer,
                &renderer.renderer2DData.quadData.indexBuffer,
                commandBuffers,
                &renderer.renderer2DData.quadData.descriptorSetLayout,
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
        if (renderer.imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
            // Wait for the fence to signal that it's available for usage. This 
            // will now ensure that there are no more than 2 frames in use, and 
            // that these frames are not accidentally using the same image!
            vkWaitForFences(renderer.deviceData.logicalDevice, 1, &renderer.imagesInFlight[imageIndex],
                VK_TRUE, UINT64_MAX);
        }
        // Now, use the image in this frame!.
        renderer.imagesInFlight[imageIndex] = renderer.inFlightFences[currentFrame];

        updateUniformBuffer(&uBuffers[0][imageIndex].bufferMemory, 
            renderer.deviceData.logicalDevice, -200.0f, 0.0f);

        updateUniformBuffer(&uBuffers[1][imageIndex].bufferMemory, 
            renderer.deviceData.logicalDevice, 200.0f, 0.0f);
        // Once we have that, we now need to submit the image to the queue:
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        
        // We need to specify which semaphores to wait on before executing the frame.
        VkSemaphore waitSemaphores[] = { renderer.imageAvailableSemaphores[currentFrame] };
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
        VkSemaphore signalSemaphores[] = {renderer.renderFinishedSemaphores[currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        // We need to reset the fence to an unsignalled state before moving on.
        vkResetFences(renderer.deviceData.logicalDevice, 1, &renderer.inFlightFences[currentFrame]);
       
        // Finally, we submit the buffer to the graphics queue 
        if (vkQueueSubmit(renderer.deviceData.graphicsQueue, 1, &submitInfo, renderer.inFlightFences[currentFrame])
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
                &renderer.renderer2DData.graphicsPipeline,
                renderer.renderer2DData.commandPool,
                renderer.renderer2DData.frameBuffers,
                &renderer.renderer2DData.quadData.vertexBuffer,
                &renderer.renderer2DData.quadData.indexBuffer,
                commandBuffers,
                &renderer.renderer2DData.quadData.descriptorSetLayout,
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
    
    // --------------------------- CLEANUP ------------------------------

    // Since our image drawing is asynchronous, we need to make sure that
    // our resources aren't in use when trying to clean them up:
    vkDeviceWaitIdle(renderer.deviceData.logicalDevice);

    VulkanUtils::cleanupSwapchain(
        renderer.deviceData.logicalDevice,
        &renderer.swapchainData,
        &renderer.renderer2DData.graphicsPipeline,
        renderer.renderer2DData.commandPool,
        renderer.renderer2DData.frameBuffers,
        commandBuffers,
        uBuffers,
        descriptorPool,
        objects
    );

    vkDestroyDescriptorSetLayout(renderer.deviceData.logicalDevice,
         renderer.renderer2DData.quadData.descriptorSetLayout,nullptr);

    // Cleans up the memory buffer 
    vkDestroyBuffer(renderer.deviceData.logicalDevice,
        renderer.renderer2DData.quadData.vertexBuffer.bufferData.buffer, nullptr);

    // Frees the allocated vertex buffer memory 
    vkFreeMemory(renderer.deviceData.logicalDevice,
        renderer.renderer2DData.quadData.vertexBuffer.bufferData.bufferMemory, nullptr);

    vkDestroyBuffer(renderer.deviceData.logicalDevice,
        renderer.renderer2DData.quadData.indexBuffer.bufferData.buffer, nullptr);

    vkFreeMemory(renderer.deviceData.logicalDevice,
        renderer.renderer2DData.quadData.indexBuffer.bufferData.bufferMemory, nullptr);

    Renderer::cleanupRenderer(&renderer, enableValidationLayers);
    
    // GLFW cleanup
    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS; 
} 
