
#ifndef WINDOW_H
#define WINDOW_H

namespace PongWindow {

	enum class NativeWindowType
	{
		NONE = 0,
		GLFW
	};

	struct WindowData {
		int width{ 0 };
		int height{ 0 };
		char* name{ "Window" };
		bool isUsingVsync{ true };
		bool isResized{ false };
		bool isRunning{ true };
	};

	struct Window {
		void* nativeWindow			{nullptr};
		NativeWindowType type		{NativeWindowType::NONE};
		WindowData windowData;
	};

	Window* initialiseWindow(NativeWindowType, int, int, char*, bool = true);
	void destroyWindow(Window*);
	void onWindowMinimised(void*, NativeWindowType, int*, int*);
	void onWindowUpdate();
	bool isWindowRunning(Window*);
}

#endif
