#include "input.h"
#include <GLFW/glfw3.h>

namespace Pong {

	bool isKeyPressed(PongWindow::Window* window, int keycode) {

	    bool status = false;

		if (window->type == PongWindow::NativeWindowType::GLFW) {
			status = glfwGetKey(static_cast<GLFWwindow*>(window->nativeWindow), keycode) == GLFW_PRESS;
		}

		return status;
	}

    bool isMouseButtonPressed(PongWindow::Window* window, int mouseButtonCode) {
        bool status = false;

        if (window->type == PongWindow::NativeWindowType::GLFW) {
            status = glfwGetMouseButton(static_cast<GLFWwindow*>(window->nativeWindow), mouseButtonCode) == GLFW_PRESS;
        }

        return status;
    }

    glm::vec2 getMousePosition(PongWindow::Window* window) {

	    double xPos, yPos = 0.0;

        if (window->type == PongWindow::NativeWindowType::GLFW) {
            glfwGetCursorPos(static_cast<GLFWwindow*>(window->nativeWindow), &xPos, &yPos);
        }

        return { (float)xPos, (float)yPos };
	}
}