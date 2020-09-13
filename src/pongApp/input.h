#ifndef INPUT_H
#define INPUT_H

#include "../window/window.h"
#include <glm/glm.hpp>

namespace Pong {

	bool isKeyPressed(PongWindow::Window*, int);
	bool isMouseButtonPressed(PongWindow::Window*, int);
	glm::vec2 getMousePosition(PongWindow::Window*);
}

#endif // !INPUT_H

