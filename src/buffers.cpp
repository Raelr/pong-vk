#include "buffers.h"

namespace Buffers {
	
	// ---------------------------- BUFFER ----------------------------------

	VkResult createBuffer(
		VkPhysicalDevice physicalDevice,
		VkDevice logicalDevice,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties,
		BufferData& bufferData
	) {
		// We need to start by creating a configuration for our vertex buffer.
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		// We need to specify the size of the buffer in bytes. 
		bufferInfo.size = size;
		// Specify what type of buffer this is
		bufferInfo.usage = usage;
		// Specifies that this will only be used by a single queue - the
		// graphics queue.
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		// Create the buffer using the create object we specified before
		if ((vkCreateBuffer(logicalDevice, &bufferInfo, nullptr,
			&bufferData.buffer)) != VK_SUCCESS) {
			return VkResult::VK_ERROR_INITIALIZATION_FAILED;
		}

		// Once the buffer has been created, we need to actually allocate memory to it.

		// First we need to get the memory requirements for the buffer
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(logicalDevice, bufferData.buffer,
			&memRequirements);

		VkMemoryAllocateInfo allocInfo{};

		// Store the memory type in a variable
		uint32_t memoryType;

		// Search for our memory requirements and check if we can map this memory from
		// our CPU to the GPU.
		if (!findMemoryType(physicalDevice, &memoryType,
			memRequirements.memoryTypeBits, properties)) {

			return VK_ERROR_INITIALIZATION_FAILED;
		}

		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		// Specify the size of our allocation using our memory requirements
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = memoryType;

		// Now allocate the memory
		if (vkAllocateMemory(logicalDevice, &allocInfo, nullptr,
			&bufferData.bufferMemory) != VK_SUCCESS) {

			return VK_ERROR_INITIALIZATION_FAILED;
		}

		// Now we associate the buffer with our memory
		vkBindBufferMemory(logicalDevice, bufferData.buffer, bufferData.bufferMemory, 0);

		return VK_SUCCESS;
	}

	void copyBuffer(
		VkQueue submitQueue,
		VkDevice device,
		VkBuffer srcBuffer,
		VkBuffer dstBuffer,
		VkDeviceSize size,
		VkCommandPool commandPool
	) {
		// Transferring memory between buffers always occurs in our command buffers.
		// Start by creating a command buffer and specify its usage.
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
		
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		// Immediately start recording the command buffer
		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		// Define how data will be transferred between buffers in a
		// CopyRegion struct. 
		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0; // optional
		copyRegion.dstOffset = 0; // optional
		// Specify the size of the buffer to be transferred
		copyRegion.size = size;
		// Copy the buffer data between the staging and destination buffer
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		// End recording
		vkEndCommandBuffer(commandBuffer);

		// Now define how the data will be submitted to the queue
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		// Submit the data to our queue (usually the graphics queue)
		vkQueueSubmit(submitQueue, 1, &submitInfo, VK_NULL_HANDLE);
		// Wait for the queue to become idle
		vkQueueWaitIdle(submitQueue);
		// Clean up the command buffer after usage
		vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
	}

	// Used to allocate memory from the GPU for a buffer.
	bool findMemoryType(
		VkPhysicalDevice physicalDevice,
		uint32_t* memoryType,
		uint32_t typeFilter,
		VkMemoryPropertyFlags properties
	) {
		bool isMemoryTypeFound = false;

		// First, we query the types of memory available to us
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		// Now lets look through the memory types and find one that suits us
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			// The type filter will be used to specify the memory types that are 
			// suitable. In our case we're looking for cases where the corresponding
			// bit is set to 1. 
			// We also need to check if the memory type supports the properties that we
			// need
			if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags
				& properties) == properties) {
				*memoryType = i;
				isMemoryTypeFound = true;
				break;
			}
		}

		return isMemoryTypeFound;
	}

	// ------------------------- VERTEX BUFFER METHODS ------------------------------

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
