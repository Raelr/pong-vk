#ifndef PONG_VK_RENDERER2D_H
#define PONG_VK_RENDERER2D_H

#include "vulkan/vulkanUtils.h"
#include <glm/glm.hpp>

namespace Renderer2D {

    struct QuadData {
        VkDescriptorSetLayout descriptorSetLayout       {VK_NULL_HANDLE};
        size_t quadCount                                {0};
        VkDescriptorSet* descriptorSets[64]             {nullptr};
        Buffers::BufferData* uniformBuffers[64]         {nullptr};
        size_t maxQuads                                 {64};
        Buffers::VertexBuffer vertexBuffer              {0};
        Buffers::IndexBuffer indexBuffer                {nullptr};
    };

    struct Renderer2DData {
        VulkanUtils::GraphicsPipelineData graphicsPipeline  { VK_NULL_HANDLE };
        QuadData quadData                                   { VK_NULL_HANDLE };
        VkFramebuffer* frameBuffers                         { VK_NULL_HANDLE };
        VkCommandPool commandPool                           { VK_NULL_HANDLE };
        VkDescriptorPool descriptorPool                     { VK_NULL_HANDLE };
    };

    bool initialiseRenderer2D(VulkanUtils::VulkanDeviceData*, Renderer2DData*, VulkanUtils::SwapchainData);
    bool queueQuad(Renderer2DData*, VulkanUtils::VulkanDeviceData*, VulkanUtils::SwapchainData*);
    void cleanupRenderer2D(VulkanUtils::VulkanDeviceData*, Renderer2DData*);
}

#endif //PONG_VK_RENDERER2D_H
