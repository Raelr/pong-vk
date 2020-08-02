#ifndef BUFFERS_H
#define BUFFERS_H

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <array>

namespace Buffers {

    // --------------------------- BUFFER STRUCT --------------------------------

    struct BufferData {

        VkBuffer buffer;
        VkDeviceMemory bufferMemory;

    };

    // -------------------------- INDEX BUFFER STRUCT ---------------------------

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

    struct VertexBuffer {
        uint32_t vertexCount;
        Vertex* vertices;
        BufferData bufferData;
    };

    // ---------------------------- BUFFER METHODS ------------------------------

    VkResult createBuffer(
        VkPhysicalDevice,
        VkDevice,
        VkDeviceSize,
        VkBufferUsageFlags,
        VkMemoryPropertyFlags,
        VkBuffer&,
        VkDeviceMemory&
    );

    void copyBuffer(
        VkQueue,
        VkDevice,
        VkBuffer,
        VkBuffer,
        VkDeviceSize,
        VkCommandPool
    );

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