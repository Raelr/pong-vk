#ifndef PONG_VK_RENDERER2D_H
#define PONG_VK_RENDERER2D_H

#include "../vulkanUtils.h"

namespace Renderer2D {

    struct QuadData {
        VkDescriptorSetLayout descriptorSetLayout       {nullptr};
        size_t quadCount                                {0};
        VkDescriptorSet descriptorSets[1024]            {nullptr};
        size_t stride                                   {3};
        size_t maxQuads                                 {10};
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
}

#endif //PONG_VK_RENDERER2D_H
