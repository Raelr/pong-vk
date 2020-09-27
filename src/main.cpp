#include <chrono>
#include "logger.h"
#include <cstdint>
#include "renderer/renderer.h"
#include "window/window.h"
#include "pongApp/input.h"
#include "pongApp/components.h"

#define PONG_FATAL_ERROR(...) PONG_ERROR(__VA_ARGS__); return EXIT_FAILURE

// TODO: Handle paddle bounce logic
// TODO: Handle scene resetting when ball hits either end of the map
// TODO: Score tracking
// TODO: Text rendering for menus + display

const float BALL_VELOCITY = 600.0f;
const float PADDLE_VELOCITY = 400.0f;

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

    size_t paddleA  {0};
    size_t paddleB  {1};
    size_t ball     {2};

    Pong::Transform transformComponents[] {
        {
            {-350.0f,-0.0f,1.0f},
            {0.0f,0.0f,1.0f},
            {30.0f,100.0f, 1.0f}, 0.0f
        },
        {
            {350.0f, -0.0f, 1.0f},
            {0.0f,0.0f,1.0f},
            {30.0f,100.0f, 1.0f}, 0.0f
        },
        {
            {0.0f, 0.0f, 1.0f},
            {0.0f,0.0f,1.0f},
            {25.0f,25.0f, 1.0f}, 0.0f
        }
    };
    Pong::Velocity velocityComponents[] {
        { {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} },
        { {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} },
        { {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} }
    };

    Pong::RectBounds rectBoundComponents[] {
        Pong::initialiseRectBounds(transformComponents[paddleA]),
        Pong::initialiseRectBounds(transformComponents[paddleB]),
        Pong::initialiseRectBounds(transformComponents[ball])
    };

    float oldTime, currentTime, deltaTime, elapsed, frames { 0.0f };

    glm::vec3 ballDirection {1.0f, 0.0f, 0.0f};

    float timeFactor{ 1.0f };

    // -------------------------- MAIN LOOP ------------------------------

    while (PongWindow::isWindowRunning(window)) {

        glm::vec2 windowSize = {
            static_cast<float>(window->windowData.width * 0.5f),
            static_cast<float>(window->windowData.height * 0.5f)
        };

        currentTime = getTime();

        PongWindow::onWindowUpdate();
        
        deltaTime = std::clamp((currentTime - oldTime) * timeFactor, 0.0f, 0.1f);
        elapsed += deltaTime;

        // Input
        if (Pong::isKeyPressed(window, KEY_W)) {
            velocityComponents[paddleA].positionVelocity.y += (PADDLE_VELOCITY * deltaTime);
        }
        if (Pong::isKeyPressed(window, KEY_S)) {
            velocityComponents[paddleA].positionVelocity.y -= (PADDLE_VELOCITY * deltaTime);
        }

        if (Pong::isKeyPressed(window, KEY_UP)) {
            velocityComponents[paddleB].positionVelocity.y += (PADDLE_VELOCITY * deltaTime);
        }
        if (Pong::isKeyPressed(window, KEY_DOWN)) {
            velocityComponents[paddleB].positionVelocity.y -= (PADDLE_VELOCITY * deltaTime);
        }

        // Game Logic
        velocityComponents[ball].positionVelocity += (BALL_VELOCITY * ballDirection * deltaTime);

        for (int i = 0; i < currentEntities; i++) {
            Pong::addVelocity(transformComponents[i], velocityComponents[i]);
            if (i != ball) {
                transformComponents[i].position = { 
                    transformComponents[i].position.x, 
                    std::clamp(transformComponents[i].position.y,-windowSize.y + (transformComponents[i].scale.y * 0.5f), 
                    windowSize.y - (transformComponents[i].scale.y * 0.5f)), 1.0f 
                };
            }
            Pong::updateRectBounds(rectBoundComponents[i], transformComponents[i]);
            velocityComponents[i].positionVelocity = glm::vec3(0.0f);
        }

        Pong::Transform& ballTransform = transformComponents[ball];
        // AABB Collisions
        for (size_t i = 0; i < currentEntities; i++) {
            if (i == ball) continue;
            if (Pong::isOverlapping(rectBoundComponents[ball], rectBoundComponents[i])) {
                glm::vec3 difference = Pong::resolveCollision(transformComponents[ball], rectBoundComponents[ball],
                    transformComponents[i], rectBoundComponents[i], ballDirection);
                transformComponents[ball].position += difference;
                float distanceFromCentre = transformComponents[ball].position.y - transformComponents[i].position.y;
                float normalised = std::clamp(distanceFromCentre / (transformComponents[i].scale.y * 0.5f), -1.0f, 1.0f);
                ballDirection.x = -ballDirection.x;
                ballDirection.y = normalised;
            }
        }

        // Handle hotizontal collisions with side of field.
        if ((rectBoundComponents[ball].maxX > static_cast<float>(window->windowData.width * 0.5f)) ||
                rectBoundComponents[ball].minX < -static_cast<float>(window->windowData.width * 0.5f)) {
            // TODO: create a score management system.

        // Handle vertical collisions with top and bottom of the map
        } else if ((rectBoundComponents[ball].maxY > static_cast<float>(window->windowData.height * 0.5f)) ||
                rectBoundComponents[ball].minY < -static_cast<float>(window->windowData.height * 0.5f)) {
            ballDirection.y = -ballDirection.y;
        }

        float paddleASize = transformComponents[paddleA].scale.y;
        float sectionSize = paddleASize / 3.0f;

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
                player.scale, {1.0f, 1.0f, 1.0f});
        }

        // Draw our frame and store the result.
        Renderer::Status renderStatus = Renderer::drawFrame(&renderer, &window->windowData.isResized);

        if (renderStatus == Renderer::Status::FAILURE) {
            PONG_ERROR("Error drawing frame - exiting main loop!");
            break;
        } else if (renderStatus == Renderer::Status::SKIPPED_FRAME) {
            timeFactor = 0.0f;
            PongWindow::onWindowMinimised(window->nativeWindow, window->type,
                &renderer.deviceData.framebufferWidth, &renderer.deviceData.framebufferHeight);
            Renderer::recreateSwapchain(&renderer);
            window->windowData.isResized = false;
            timeFactor = 1.0f;
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
