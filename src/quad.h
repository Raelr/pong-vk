#ifndef QUAD_H 
#define QUAD_H

#include <vulkan/vulkan.h>
#include "buffers.h"
#include <glm/glm.hpp>

static Buffers::Vertex s_quadVertices[] = {
    // Positions        // Colors
    { {-0.5f, -0.5f},   {1.0f, 1.0f, 1.0f} },
    { {0.5f, -0.5f},    {1.0f, 1.0f, 1.0f} },
    { {0.5f, 0.5f},     {1.0f, 1.0f, 1.0f} },
    { {-0.5f, 0.5f},    {1.0f, 1.0f, 1.0f} }
};


struct Quad {
	Buffers::Vertex* vertices = s_quadVertices;
    glm::vec2 m_pos;
    Buffers::UniformBufferObject* uniformBuffers;
    VkDescriptorSet* descriptorSets;
};

#endif
