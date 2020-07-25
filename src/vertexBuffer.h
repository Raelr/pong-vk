#ifndef VERTEX_BUFFER_H
#define VERTEX_BUFFER_H

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <array>

namespace VertexBuffer {

	struct Vertex {
		glm::vec2 pos;
		glm::vec3 color;
	};

	VkVertexInputBindingDescription getBindingDescription();

	std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();
}

#endif // !VERTEXT_BUFFERS_H

