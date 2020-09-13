#define GLFW_INCLUDE_VULKAN
#include <glm/glm.hpp>
#include <chrono>
#include "logger.h"
#include <cstdint>
#include "renderer/renderer.h"
#include "window/window.h"

#define PONG_FATAL_ERROR(...) ERROR(__VA_ARGS__); return EXIT_FAILURE

// Only enable validation layers when the program is run in DEBUG mode.
#ifdef DEBUG
    const bool enableValidationLayers = true;
#else
    const bool enableValidationLayers = false;
#endif

float getTime() {
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();

    return std::chrono::duration<float,
            std::chrono::seconds::period>(currentTime - startTime).count();
}

struct PlayerData {
    glm::vec3 position = { 0.0f, 0.0f, 0.0f };
    glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };
    glm::vec3 scale = { 0.0f, 0.0f, 0.0f };
    float rotationAngle = 0.f;
    uint32_t playerIndex = 0;
};

int main() {  

    // -------------------- INITIALISE WINDOW --------------------------

    initLogger();

    // Initialise the window struct
    auto window = PongWindow::initialiseWindow(PongWindow::NativeWindowType::GLFW, 800, 600, "Pong");

    INFO("Created GLFW window");

    // ============================ RENDERER =================================

    // ----------------------- INITIALISE RENDERER ---------------------------

    Renderer::Renderer renderer;

    // Let the renderer know that we want to load in default validation layers
    Renderer::loadDefaultValidationLayers(&renderer);
    Renderer::loadDefaultDeviceExtensions(&renderer);

    if (Renderer::initialiseRenderer(&renderer, enableValidationLayers, window->nativeWindow, 
        Renderer::WindowType::GLFW) != Renderer::Status::SUCCESS) {
        PONG_FATAL_ERROR("Failed to initialise renderer!");
    }

    // ------------------------ SCENE SETUP -----------------------------

    //Renderer::registerQuad2D(&renderer);

    PlayerData players[64];
    uint32_t currentPlayers = 5;

    players[0] = {
        {-250.0f,-150.0f,1.0f},
        {0.0f,0.0f,1.0f},
        {200.0f,200.0f, 0.0f}, -90.0f, 0
    };
    players[1] = {
            {250.0f, -150.0f, 1.0f},
            {0.0f,0.0f,1.0f},
            {200.0f,200.0f, 0.0f}, -90.0f, 1
    };
    players[2] = {
            {-250.0f, 150.0f, 1.0f},
            {0.0f,0.0f,1.0f},
            {200.0f,200.0f, 0.0f}, 90.0f, 2
    };
    players[3] = {
            {250.0f, 150.0f, 1.0f},
            {0.0f,0.0f,1.0f},
            {200.0f,200.0f, 0.0f}, 90.0f, 3
    };
    players[4] = {
            {0.0f, 0.0f, 1.0f},
            {0.0f,0.0f,1.0f},
            {200.0f,200.0f, 0.0f}, 90.0f, 4
    };

    float oldTime, currentTime, deltaTime, elapsed, frames = 0.0f;

    // -------------------------- MAIN LOOP ------------------------------

    while (PongWindow::isWindowRunning(window)) {
        PongWindow::onWindowUpdate();

        // Refresh the renderer and start again next frame
        renderer.renderer2DData.quadData.quadCount = 0;

        currentTime = getTime();
        deltaTime = currentTime - oldTime;
        elapsed += deltaTime;

        // Basic FPS counter
        if (elapsed > 1.0f) {
            INFO("FRAMES: {0}", frames);
            frames = 0;
            elapsed = 0;
        }

        for (int i = 0; i < currentPlayers; i++) {
            PlayerData& player = players[i];
            Renderer::drawQuad(
                &renderer,                                      // Renderer
                player.position,                                // Position
                player.rotation,                                // Rotation
                getTime() * glm::radians(player.rotationAngle), // angle (radians)
                player.scale);                                  // Scale
        }

        Renderer::Status renderStatus = Renderer::drawFrame(&renderer, &window->windowData.isResized);

        if (renderStatus == Renderer::Status::FAILURE) {
            ERROR("Error drawing frame - exiting main loop!");
            break;
        }
        else if (renderStatus == Renderer::Status::SKIPPED_FRAME) {
            
            PongWindow::onWindowMinimised(window->nativeWindow, window->type, 
                &renderer.deviceData.framebufferWidth, &renderer.deviceData.framebufferHeight);
            Renderer::recreateSwapchain(&renderer);

            window->windowData.isResized = false;
        }

        oldTime = currentTime;
        frames++;
    }
    
    // --------------------------- CLEANUP ------------------------------

    Renderer::cleanupRenderer(&renderer, enableValidationLayers);

    // GLFW cleanup
    PongWindow::destroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS; 
} 
