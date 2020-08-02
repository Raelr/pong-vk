#ifndef BUFFERS_H
#define BUFFERS_H

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <array>

namespace Buffers {

    // --------------------------- BUFFER STRUCT --------------------------------

    // Simple struct for storing buffer data.
    struct BufferData {
        VkBuffer buffer;
        VkDeviceMemory bufferMemory;
    };

    // -------------------------- INDEX BUFFER STRUCT ---------------------------

    // Data necessary for using an index buffer
    struct IndexBuffer {
        uint16_t* indices;
        uint32_t indexCount;
        BufferData bufferData;
    };

    // -------------------------- VERTEX BUFFER STRUCT---------------------------

    // A simple struct for storing vertex data. May need to split between a 
    // 3D and 2D vertex at some point.
    struct Vertex {
        // position vector
        glm::vec2 pos;
        // Color vector
        glm::vec3 color;
    };

    // Data necessary to store a vertex buffer
    struct VertexBuffer {
        uint32_t vertexCount;
        Vertex* vertices;
        BufferData bufferData;
    };

    // ---------------------------- BUFFER METHODS ------------------------------

    // A method for creating a generic buffer - to be used for buffer creation
    VkResult createBuffer(
        VkPhysicalDevice,
        VkDevice,
        VkDeviceSize,
        VkBufferUsageFlags,
        VkMemoryPropertyFlags,
        BufferData&
    );

    // Used to copy data between a staging buffer and a standard buffer (index or vertex)
    void copyBuffer(
        VkQueue,
        VkDevice,
        VkBuffer,
        VkBuffer,
        VkDeviceSize,
        VkCommandPool
    );

    // Used to actually allocate memory from the GPU for a buffer.
    bool findMemoryType(
        VkPhysicalDevice,
        uint32_t*,
        uint32_t,
        VkMemoryPropertyFlags
    );

    // ------------------------ VERTEX BUFFER METHODS -------------------------

    // Returns vertex binding information. This information specifies the rate
    // with which data is loaded from memory throughout the vertices. 
    VkVertexInputBindingDescription getBindingDescription();

    // Return vertex input attributes. This describes how a vertex attribute
    // can be extracted from a chunk of vertex data. 
    std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();

}

#endif // !BUFFERS_H