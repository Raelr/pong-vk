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
            getTime() * glm::radians(-90.0f), {200.0f, 200.0f, 0.0f}, playerOne);
        Renderer::drawQuad(&renderer, {200.0f, -125.0f, 1.0f}, {0.0f, 0.0f, 1.0f},
            getTime() * glm::radians(-90.0f), {200.0f, 200.0f, 0.0f}, playerTwo);
        Renderer::drawQuad(&renderer, {0.0f, 100.0f, 1.0f}, {0.0f, 0.0f, 1.0f},
            getTime() * glm::radians(90.0f), {200.0f, 200.0f, 0.0f}, playerThree);

        Renderer::drawFrame(&renderer, &pongData.framebufferResized, window);
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
