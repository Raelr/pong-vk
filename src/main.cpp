#include <chrono>
#include "logger.h"
#include <cstdint>
#include "renderer/renderer.h"
#include "window/window.h"
#include "pongApp/input.h"
#include "pongApp/components.h"

#define PONG_FATAL_ERROR(...) PONG_ERROR(__VA_ARGS__); return EXIT_FAILURE

const float BALL_VELOCITY = 500.0f;

// TODO: Move these to a separate file
#define KEY_W GLFW_KEY_W
#define KEY_A GLFW_KEY_A
#define KEY_S GLFW_KEY_S
#define KEY_D GLFW_KEY_D
#define KEY_UP GLFW_KEY_UP
#define KEY_DOWN GLFW_KEY_DOWN
#define MOUSE_LMB GLFW_MOUSE_BUTTON_1
#define MOUSE_RMB GLFW_MOUSE_BUTTON_2

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

    uint32_t currentEntities = 3;

    Pong::Transform transformComponents[3];
    Pong::Velocity velocityComponents[] {
        { {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} },
        { {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} },
        { {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} }
    };

    transformComponents[0] = {
        {-350.0f,-150.0f,1.0f},
        {0.0f,0.0f,1.0f},
        {50.0f,100.0f, 0.0f}, 0.0f
    };
    transformComponents[1] = {
        {350.0f, -150.0f, 1.0f},
        {0.0f,0.0f,1.0f},
        {50.0f,100.0f, 0.0f}, 0.0f
    };
    transformComponents[2] = {
            {0.0f, 0.0f, 1.0f},
            {0.0f,0.0f,1.0f},
            {25.0f,25.0f, 0.0f}, 0.0f
    };

    Pong::RectAABB colliders[] {
            Pong::initialiseAABBCollision(transformComponents[0]),
            Pong::initialiseAABBCollision(transformComponents[1]),
            Pong::initialiseAABBCollision(transformComponents[2])
    };

    PONG_INFO("minX {0} minY {1} maxX {2} maxY {3}", colliders[0].minX, colliders[0].minY, colliders[0].maxX, colliders[0].maxY);

    size_t paddleA  {0};
    size_t paddleB  {1};
    size_t ball     {3};

    float oldTime, currentTime, deltaTime, elapsed, frames = 0.0f;

    // -------------------------- MAIN LOOP ------------------------------

    while (PongWindow::isWindowRunning(window)) {
        PongWindow::onWindowUpdate();

        // Time
        currentTime = getTime();
        deltaTime = currentTime - oldTime;
        elapsed += deltaTime;

        // Input
        if (Pong::isKeyPressed(window, KEY_W)) {
            velocityComponents[paddleA].positionVelocity.y += (300.0 * deltaTime);
        }
        if (Pong::isKeyPressed(window, KEY_S)) {
            velocityComponents[paddleA].positionVelocity.y -= (300.0 * deltaTime);
        }
        if (Pong::isKeyPressed(window, KEY_UP)) {
            velocityComponents[paddleB].positionVelocity.y += (300.0 * deltaTime);
        }
        if (Pong::isKeyPressed(window, KEY_DOWN)) {
            velocityComponents[paddleB].positionVelocity.y -= (300.0 * deltaTime);
        }

        for (int i = 0; i < currentEntities; i++) {
            Pong::addVelocity(transformComponents[i], velocityComponents[i]);
        }

        // Basic FPS counter
        if (elapsed > 1.0f) {
            PONG_TRACE("FRAMES: {0}", frames);
            frames = 0;
            elapsed = 0;
        }

        // Render Frame
        for (int i = 0; i < currentEntities; i++) {
            Pong::Transform& player = transformComponents[i];
            Renderer::drawQuad(&renderer, player.position, player.rotation, glm::radians(player.rotationAngle),
                player.scale);
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
