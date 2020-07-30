#ifndef VERTEX_BUFFER_H
#define VERTEX_BUFFER_H

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <array>

namespace VertexBuffer {

	// A simple struct for storing vertex data. May need to split between a 
	// 3D and 2D vertex at some point.
	struct Vertex {
		// position vector
		glm::vec2 pos;
		// Color vector
		glm::vec3 color;
	};

    struct VertexBuffer {
        uint32_t vertexCount;
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;
        Vertex* vertices;
    };

    struct IndexBuffer {
        
        uint16_t* indices;
        uint32_t indexCount;
        VkBuffer indexBuffer;
        VkDeviceMemory indexBufferMemory;
    };

	// Returns vertex binding information. This information specifies the rate
	// with which data is loaded from memory throughout the vertices. 
	VkVertexInputBindingDescription getBindingDescription();

	// Return vertex input attributes. This describes how a vertex attribute
	// can be extracted from a chunk of vertex data. 
	std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();
}

#endif // !VERTEXT_BUFFERS_H

