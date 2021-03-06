#ifndef PONG_VK_RENDERER2D_H
#define PONG_VK_RENDERER2D_H

#include "vk/vulkanUtils.h"
#include <glm/glm.hpp>
#include "vk/vulkanDeviceData.h"
#include "core.h"
#include "vk/texture2d.h"

namespace Renderer2D {

    struct QuadProperties {
        glm::mat4 mvp;
        glm::vec3 color;
    };

    struct QuadData {
        VkDescriptorSetLayout descriptorSetLayout                   {VK_NULL_HANDLE};
        size_t quadCount                                            {0};
        size_t maxQuads                                             {256};
        Buffers::VertexBuffer vertexBuffer                          {0};
        Buffers::IndexBuffer indexBuffer                            {nullptr};
        Buffers::DynamicUniformBuffer<QuadProperties> dynamicData   {0};
        VkDescriptorSet* dynamicDescriptorSets                      {nullptr};
        Renderer::Texture2D texture                                 {0};
    };

    struct Renderer2DData {
        Renderer::GraphicsPipelineData graphicsPipeline         { VK_NULL_HANDLE };
        QuadData quadData                                       { VK_NULL_HANDLE };
        VkFramebuffer* frameBuffers                             { VK_NULL_HANDLE };
        VkCommandPool commandPool                               { VK_NULL_HANDLE };
        VkDescriptorPool descriptorPool                         { VK_NULL_HANDLE };
        VkCommandBuffer* commandBuffers                         {nullptr};
    };

    bool initialiseRenderer2D(Renderer::VulkanDeviceData*, Renderer2DData*, Renderer::SwapchainData);
    void cleanupRenderer2D(Renderer::VulkanDeviceData*, Renderer2DData*);
    bool recreateRenderer2D(Renderer::VulkanDeviceData* deviceData, Renderer2DData* renderer2D,
        Renderer::SwapchainData swapchain);
}

#endif //PONG_VK_RENDERER2D_H
