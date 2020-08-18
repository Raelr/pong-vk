#ifndef PONG_VK_RENDERER2D_H
#define PONG_VK_RENDERER2D_H

#include "../vulkanUtils.h"

namespace Renderer2D {

    struct QuadData {
        VkDescriptorSetLayout descriptorSetLayout       {nullptr};
        size_t quadCount                                {0};
        VkDescriptorSet descriptorSets[64]            {nullptr};
        Buffers::BufferData* uniformBuffers[64]       {nullptr};
        size_t stride                                   {3};
        size_t maxQuads                                 {20};
        Buffers::VertexBuffer vertexBuffer              {0};
        Buffers::IndexBuffer indexBuffer                {nullptr};
    };

    struct Renderer2DData {
        VulkanUtils::GraphicsPipelineData graphicsPipeline  {nullptr};
        QuadData quadData                                   {nullptr};
        VkFramebuffer *frameBuffers                         {nullptr};
        VkCommandPool commandPool                           {nullptr};
    };

    bool initialiseRenderer2D(VulkanUtils::VulkanDeviceData*, Renderer2DData*, VulkanUtils::SwapchainData);
    bool drawQuad2D();
}

#endif //PONG_VK_RENDERER2D_H
