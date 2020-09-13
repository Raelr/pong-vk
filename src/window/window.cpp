#include "window.h"
#include <GLFW/glfw3.h>
#include "../logger.h"

namespace PongWindow {

	Window* initialiseWindow(NativeWindowType type, int width, int height, char* name,
		bool isUsingVsync) {

		Window* window = new Window();
		window->windowData.name = name;
		window->windowData.width = width;
		window->windowData.height = height;
		window->windowData.isUsingVsync = isUsingVsync;
		window->type = type;

		if (type == NativeWindowType::GLFW) {
			// Initialise GLFW
			glfwInit();

			// Set flags for the window (set it to not use GLFW API and 
			// set it to not resize)
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

			// Actually make the window
			GLFWwindow* glfwWindow = glfwCreateWindow(width, height, name,
				nullptr, nullptr);

			glfwSetWindowUserPointer(glfwWindow, &window->windowData);

			glfwSetFramebufferSizeCallback(glfwWindow, [](GLFWwindow* window, int width,
				int height) {
				auto data = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
				data->isResized = true;
			});

			glfwSetWindowCloseCallback(glfwWindow, [](GLFWwindow* window) {
				auto data = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
				data->isRunning = false;
			});

			window->nativeWindow = glfwWindow;
		}

		return window;
	}

	void destroyWindow(Window* window) {
		if (window->type == NativeWindowType::GLFW) {
			glfwDestroyWindow(static_cast<GLFWwindow*>(window->nativeWindow));
			delete window;
			glfwTerminate();
		}
	}

	// Handles a case where the window is minimised - pauses rendering until its opened again.
	void onWindowMinimised(void* nativeWindow, NativeWindowType type, int* width, int* height) {

		if (type == NativeWindowType::GLFW) {
			auto window = static_cast<GLFWwindow*>(nativeWindow);
			glfwGetFramebufferSize(window, width, height);

			// If the window is minimized we simply pause rendering until it comes back!
			while (*width == 0 && *height == 0) {
				glfwGetFramebufferSize(window, width, height);
				glfwWaitEvents();
			}
		}
	}

	void onWindowUpdate() {
		glfwPollEvents();
	}

	bool isWindowRunning(Window* window) {
		return window->windowData.isRunning;
	}
}