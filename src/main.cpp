#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <cstdlib>

// Const variables to determine initial window height ad 
const int WINDOW_HEIGHT = 600;
const int WINDOW_WIDTH = 800;

int main() {  
    // Initialise GLFW
    glfwInit();

    // Set flags for the window (set it to not use GLFW API and 
    // set it to not resize)
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // Actually make the window
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Pong", nullptr, nullptr);

    // Start the loop and only stop when a close event happens.
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    // Cleaning up memory
    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS; 
} 