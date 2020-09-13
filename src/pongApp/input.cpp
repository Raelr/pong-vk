#include "input.h"
#include <GLFW/glfw3.h>

namespace Pong {

	bool getKeyDown(PongWindow::Window* window, int keycode) {

		if (window->type == PongWindow::NativeWindowType::GLFW) {
			auto status = glfwGetKey(static_cast<GLFWwindow*>(window->nativeWindow), keycode);
			return status == GLFW_PRESS;
		}
	}

}