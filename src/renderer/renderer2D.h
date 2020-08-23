#ifndef PONG_VK_RENDERER2D_H
#define PONG_VK_RENDERER2D_H

#include "../vulkanUtils.h"
#include <glm/glm.hpp>

namespace Renderer2D {

    struct QuadData {
        VkDescriptorSetLayout descriptorSetLayout       {nullptr};
        size_t quadCount                                {0};
        VkDescriptorSet* descriptorSets[64]             {nullptr};
        Buffers::BufferData* uniformBuffers[64]         {nullptr};
        size_t maxQuads                                 {64};
        Buffers::VertexBuffer vertexBuffer              {0};
        Buffers::IndexBuffer indexBuffer                {nullptr};
    };

    struct Renderer2DData {
        VulkanUtils::GraphicsPipelineData graphicsPipeline  {nullptr};
        QuadData quadData                                   {nullptr};
        VkFramebuffer* frameBuffers                         {nullptr};
        VkCommandPool commandPool                           {nullptr};
        VkDescriptorPool descriptorPool                     {nullptr};
    };

    bool initialiseRenderer2D(VulkanUtils::VulkanDeviceData*, Renderer2DData*, VulkanUtils::SwapchainData);
    bool queueQuad(Renderer2DData*, VulkanUtils::VulkanDeviceData*, VulkanUtils::SwapchainData*, Buffers::UniformBufferObject);
    bool drawQuads();
}

#endif //PONG_VK_RENDERER2D_H
