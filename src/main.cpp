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

float getTime() {
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();

    return std::chrono::duration<float,
            std::chrono::seconds::period>(currentTime - startTime).count();
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

    Renderer::registerQuad2D(&renderer);
    Renderer::registerQuad2D(&renderer);
    Renderer::registerQuad2D(&renderer);

    uint32_t playerOne = 0;
    uint32_t playerTwo = 1;
    uint32_t playerThree = 2;


    // ------------------------- MAIN LOOP ------------------------------

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        Renderer::drawQuad(&renderer, {-200.0f, -125.0f, 1.0f}, {0.0f, 0.0f, 1.0f},
                           getTime() * glm::radians(90.0f), {200.0f, 200.0f, 0.0f}, playerOne);
        Renderer::drawQuad(&renderer, {200.0f, -125.0f, 1.0f}, {0.0f, 0.0f, 1.0f},
                           getTime() * glm::radians(-90.0f), {200.0f, 200.0f, 0.0f}, playerTwo);
        Renderer::drawQuad(&renderer, {0.0f, 100.0f, 1.0f}, {0.0f, 0.0f, 1.0f},
                           0.0f, {200.0f, 200.0f, 0.0f}, playerThree);

        // This function takes an array of fences and waits for either one or all
        // of them to be signalled. The fourth parameter specifies that we're 
        // waiting for all fences to be signalled before moving on. The last 
        // parameter takes a timeout period which we set really high (effectively
        // making it null)
        vkWaitForFences(renderer.deviceData.logicalDevice, 1,
            &renderer.inFlightFences[renderer.currentFrame], VK_TRUE, UINT64_MAX);

        // In each frame of the main loop, we'll need to perform the following
        // operations:
        // 1. acquire an image from the swapchain.
        // 2. execute the command buffer with that image as an attachment.
        // 3. Return the image to the swapchain for presentation.

        // First we need to acquire an image from the swapchain. For this we need
        // our logical device and swapchain. The third parameter is a timeout period
        // which we disable using the max of a 64-bit integer. Next we provide our
        // semaphore, and finally a variable to output the image index to. 
        VkResult result = vkAcquireNextImageKHR(renderer.deviceData.logicalDevice,
            renderer.swapchainData.swapchain, UINT64_MAX, renderer.imageAvailableSemaphores[renderer.currentFrame],
            VK_NULL_HANDLE, &renderer.imageIndex);

        if (vkGetFenceStatus(renderer.deviceData.logicalDevice, renderer.inFlightFences[renderer.currentFrame])
            == VK_SUCCESS) {

            vkResetCommandBuffer(renderer.commandBuffers[renderer.imageIndex], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

            // TODO: Change this to just accept a renderer.
            if (VulkanUtils::createCommandBuffer(renderer.deviceData.logicalDevice,
                &renderer.commandBuffers[renderer.imageIndex], renderer.imageIndex, &renderer.renderer2DData.graphicsPipeline, &renderer.swapchainData,
                renderer.renderer2DData.frameBuffers, &renderer.renderer2DData.commandPool,
                &renderer.renderer2DData.quadData.vertexBuffer, &renderer.renderer2DData.quadData.indexBuffer,
                renderer.renderer2DData.quadData.descriptorSets, renderer.renderer2DData.quadData.quadCount) != VK_SUCCESS) {

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
                renderer.commandBuffers,
                &renderer.renderer2DData.quadData.descriptorSetLayout,
                &renderer.renderer2DData.descriptorPool,
                renderer.renderer2DData.quadData.descriptorSets,
                renderer.renderer2DData.quadData.uniformBuffers,
                renderer.renderer2DData.quadData.quadCount
            );
            
            pongData.framebufferResized = false;
            // Go to the next iteration of the loop
            continue;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            PONG_FATAL_ERROR("Failed to acquire swapchain image!");
        }
        
        // Check if a previous frame is using this image. I.e: we're waiting on 
        // it's fence to be signalled. 
        if (renderer.imagesInFlight[renderer.imageIndex] != VK_NULL_HANDLE) {
            // Wait for the fence to signal that it's available for usage. This 
            // will now ensure that there are no more than 2 frames in use, and 
            // that these frames are not accidentally using the same image!
            vkWaitForFences(renderer.deviceData.logicalDevice, 1, &renderer.imagesInFlight[renderer.imageIndex],
                VK_TRUE, UINT64_MAX);
        }
        // Now, use the image in this frame!.
        renderer.imagesInFlight[renderer.imageIndex] = renderer.inFlightFences[renderer.currentFrame];

        // Once we have that, we now need to submit the image to the queue:
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        
        // We need to specify which semaphores to wait on before executing the frame.
        VkSemaphore waitSemaphores[] = { renderer.imageAvailableSemaphores[renderer.currentFrame] };
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
        submitInfo.pCommandBuffers = &renderer.commandBuffers[renderer.imageIndex];
        // Now we specify which semaphores we need to signal once our command buffers
        // have finished execution. 
        VkSemaphore signalSemaphores[] = {renderer.renderFinishedSemaphores[renderer.currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        // We need to reset the fence to an unsignalled state before moving on.
        vkResetFences(renderer.deviceData.logicalDevice, 1, &renderer.inFlightFences[renderer.currentFrame]);
       
        // Finally, we submit the buffer to the graphics queue 
        if (vkQueueSubmit(renderer.deviceData.graphicsQueue, 1, &submitInfo, renderer.inFlightFences[renderer.currentFrame])
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
        presentInfo.pImageIndices = &renderer.imageIndex;
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
                renderer.commandBuffers,
                &renderer.renderer2DData.quadData.descriptorSetLayout,
                &renderer.renderer2DData.descriptorPool,
                renderer.renderer2DData.quadData.descriptorSets,
                renderer.renderer2DData.quadData.uniformBuffers,
                renderer.renderer2DData.quadData.quadCount);

            pongData.framebufferResized = false;
        } else if (result != VK_SUCCESS) {
            PONG_FATAL_ERROR("Failed to present swapchain image!");
        }
        
        // This should clamp the value of currentFrame between 0 and 1.
        renderer.currentFrame = (renderer.currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
        renderer.renderer2DData.quadData.lastQuadCount = renderer.renderer2DData.quadData.quadCount;
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
        renderer.commandBuffers,
        renderer.renderer2DData.quadData.uniformBuffers,
        renderer.renderer2DData.descriptorPool,
        renderer.renderer2DData.quadData.quadCount
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
