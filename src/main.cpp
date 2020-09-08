#define GLFW_INCLUDE_VULKAN
#include <glm/glm.hpp>
#include <chrono>
#include <GLFW/glfw3.h>
#include "logger.h"
#include <cstdint>
#include "renderer/renderer.h"

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

// Struct for storing application-specific data. Will be used by glfw for data
// storage in the user pointer.
struct AppData {
    // Boolean for checking if the window has been resized
    bool framebufferResized = false;
};

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

    // Initialise app data struct
    AppData pongData{};

    // Initialise GLFW
    glfwInit();

    // Set flags for the window (set it to not use GLFW API and 
    // set it to not resize)
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // Actually make the window
    GLFWwindow* window = glfwCreateWindow(800, 600, "Pong",
        nullptr, nullptr);

    glfwSetWindowUserPointer(window, &pongData);

    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, 
        int height) {

        auto data =
            reinterpret_cast<AppData*>(glfwGetWindowUserPointer(window));

        data->framebufferResized = true;
    });

    INFO("Created GLFW window");

    // ============================ RENDERER =================================

    // ----------------------- INITIALISE RENDERER ---------------------------

    Renderer::Renderer renderer;

    // Let the renderer know that we want to load in default validation layers
    Renderer::loadDefaultValidationLayers(&renderer);
    Renderer::loadDefaultDeviceExtensions(&renderer);

    if (Renderer::initialiseRenderer(&renderer, enableValidationLayers, window)
        != Renderer::Status::SUCCESS) {
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

    float oldTime = 0.0f;
    float currentTime = 0.0f;
    float deltaTime = 0.0f;
    float elapsed = 0.0f;

    float frames = 0.0f;

    // -------------------------- MAIN LOOP ------------------------------

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Refresh the renderer and start again next frame
        renderer.renderer2DData.quadData.quadCount = 0;

        currentTime = getTime();
        deltaTime = currentTime - oldTime;
        elapsed += deltaTime;

        // Basic FPS counter
        if (elapsed > 1.0f) {
            INFO("FRAMES: " + std::to_string(frames));
            frames = 0;
            elapsed = 0;
        }

        for (int i = 0; i < currentPlayers; i++) {
            PlayerData& player = players[i];
            Renderer::drawQuad(&renderer, player.position, player.rotation,
                getTime() * glm::radians(player.rotationAngle), player.scale);
        }

        if (Renderer::drawFrame(&renderer, &pongData.framebufferResized, window)
            == Renderer::Status::FAILURE) {
            ERROR("Error drawing frame - exiting main loop!");
            break;
        }

        oldTime = currentTime;
        frames++;
    }
    
    // --------------------------- CLEANUP ------------------------------

    Renderer::cleanupRenderer(&renderer, enableValidationLayers);

    // GLFW cleanup
    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS; 
} 
