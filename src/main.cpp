#include <chrono>
#include "logger.h"
#include <cstdint>
#include "renderer/renderer.h"
#include "window/window.h"
#include "pongApp/input.h"

#define PONG_FATAL_ERROR(...) PONG_ERROR(__VA_ARGS__); return EXIT_FAILURE

// TODO: Move these to a separate file
#define KEY_W GLFW_KEY_W
#define KEY_A GLFW_KEY_A
#define KEY_S GLFW_KEY_S
#define KEY_D GLFW_KEY_D
#define KEY_UP GLFW_KEY_UP
#define KEY_DOWN GLFW_KEY_DOWN

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

    // ----------------------- INITIALISE WINDOW -----------------------------

    initLogger();

    // Initialise the window struct
    auto window = PongWindow::initialiseWindow(PongWindow::NativeWindowType::GLFW, 800, 600, "Pong");

    PONG_INFO("Created GLFW window");

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

    PlayerData players[64];
    uint32_t currentPlayers = 2;

    players[0] = {
        {-350.0f,-150.0f,1.0f},
        {0.0f,0.0f,1.0f},
        {50.0f,100.0f, 0.0f}, 0.0f, 0
    };
    players[1] = {
        {350.0f, -150.0f, 1.0f},
        {0.0f,0.0f,1.0f},
        {50.0f,100.0f, 0.0f}, 0.0f, 1
    };

    float oldTime, currentTime, deltaTime, elapsed, frames = 0.0f;

    // -------------------------- MAIN LOOP ------------------------------

    while (PongWindow::isWindowRunning(window)) {
        PongWindow::onWindowUpdate();

        // Time
        currentTime = getTime();
        deltaTime = currentTime - oldTime;
        elapsed += deltaTime;

        // Input
        if (Pong::getKeyDown(window, KEY_W)) {
            players[0].position.y += (300.0 * deltaTime);
        }
        if (Pong::getKeyDown(window, KEY_S)) {
            players[0].position.y -= (300.0 * deltaTime);
        }
        if (Pong::getKeyDown(window, KEY_UP)) {
            players[1].position.y += (300.0 * deltaTime);
        }
        if (Pong::getKeyDown(window, KEY_DOWN)) {
            players[1].position.y -= (300.0 * deltaTime);
        }

        // Basic FPS counter
        if (elapsed > 1.0f) {
            PONG_TRACE("FRAMES: {0}", frames);
            frames = 0;
            elapsed = 0;
        }

        // Render Frame
        for (int i = 0; i < currentPlayers; i++) {
            PlayerData& player = players[i];
            Renderer::drawQuad(
                &renderer,                                      // Renderer
                player.position,                                // Position
                player.rotation,                                // Rotation
                glm::radians(player.rotationAngle),             // angle (radians)
                player.scale);                    // Scale
        }

        // Draw our frame and store the result.
        Renderer::Status renderStatus = Renderer::drawFrame(&renderer, &window->windowData.isResized);

        if (renderStatus == Renderer::Status::FAILURE) {
            PONG_ERROR("Error drawing frame - exiting main loop!");
            break;
        } else if (renderStatus == Renderer::Status::SKIPPED_FRAME) {
            PongWindow::onWindowMinimised(window->nativeWindow, window->type,
                &renderer.deviceData.framebufferWidth, &renderer.deviceData.framebufferHeight);
            Renderer::recreateSwapchain(&renderer);
            window->windowData.isResized = false;
        }

        oldTime = currentTime;
        frames++;

        Renderer::flushRenderer(&renderer);
    }
    
    // --------------------------- CLEANUP ------------------------------

    Renderer::cleanupRenderer(&renderer, enableValidationLayers);

    // GLFW cleanup
    PongWindow::destroyWindow(window);

    return EXIT_SUCCESS; 
} 
