#include "vertexBuffer.h"

namespace VertexBuffer {
	
	// Returns vertex binding information. This information specifies the rate
	// with which data is loaded from memory throughout the vertices. 
	// TODO: make this more general so we can have more flexible
	// vertex buffers
	VkVertexInputBindingDescription getBindingDescription() {

		VkVertexInputBindingDescription bindingDescription{};
		// Specifies the index of the binding in the binding array (in case
		// there are multiple)
		bindingDescription.binding = 0;
		// Specifies that we'll move to a new data entry after each vertex.
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		// Specifies the number of bytes from one entry to the next.
		bindingDescription.stride = sizeof(Vertex);

		return bindingDescription;
	}

	// Return vertex input attributes. This describes how a vertex attribute
	// can be extracted from a chunk of vertex data. 
	std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {

		// Since we have two attributes in our vertex shader, we need two
		// attribute descriptions

		// Position input vector
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions;
		attributeDescriptions[0].binding = 0;
		// location in the shader
		attributeDescriptions[0].location = 0;
		// Describes the type of data. Since our vertex uses a 2D vector, we 
		// specify that as using the Reg and Green channels of color 
		// (its a little confusing)
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		// Defines the byte size of the data in the vertex. 
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		// Color input vector
		attributeDescriptions[1].binding = 0;
		// Location in the shader
		attributeDescriptions[1].location = 1;
		// Same as above, but with 3 dimensions
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		// Defines the byte size of the data in the vertex. 
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		return attributeDescriptions;
	}
}

