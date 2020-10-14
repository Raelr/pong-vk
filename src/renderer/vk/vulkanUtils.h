#ifndef VULKAN_UTILS_H
#define VULKAN_UTILS_H

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>
#include "../utils.h"
#include "buffers.h"
#include "vulkanDeviceData.h"
#include "swapchainData.h"

namespace Renderer {

    struct GraphicsPipelineData {
        VkRenderPass renderPass;
        VkPipeline graphicsPipeline;
        VkPipelineLayout pipelineLayout;
    };
    
    void destroySwapchainImageData(SwapchainData);

    VkResult createRenderPass(VkDevice, VkFormat format, GraphicsPipelineData*);

    VkResult createDescriptorSetLayout(
        VkDevice device,
        VkDescriptorSetLayout* descriptorSetLayout,
        VkDescriptorSetLayoutBinding* layoutBindings,
        uint32_t layoutBindingCount
    );

    VkResult createDescriptorPool(
        VkDevice,
        uint32_t,
        VkDescriptorPool*,
        VkDescriptorPoolSize*,
        uint32_t
    );

    VkResult createGraphicsPipeline(
        VkDevice, 
        GraphicsPipelineData*, 
        const SwapchainData*,
        VkDescriptorSetLayout* descriptorSetLayout
    );

    VkShaderModule createShaderModule(
        FileContents buffer, 
        VkDevice &device 
    );

    VkResult createFramebuffer(
        VkDevice device, 
        VkFramebuffer* pFramebuffers,
        SwapchainData* swapchain,
        GraphicsPipelineData* graphicsPipeline
    );

    VkResult createCommandBuffers(
        VkDevice device,
        VkCommandBuffer* buffers,
        GraphicsPipelineData* pGraphicsPipeline,
        SwapchainData* pSwapchain,
        VkFramebuffer* pFramebuffers,
        VkCommandPool commandPool,
        Buffers::VertexBuffer*,
        Buffers::IndexBuffer*,
        VkDescriptorSet* descriptorSets,
        size_t objectCount,
        uint32_t dynamicAlignment
    );

    VkResult rerecordCommandBuffer(
        VkDevice device, VkCommandBuffer* buffer, size_t bufferIndex,
        GraphicsPipelineData* pGraphicsPipeline, SwapchainData* pSwapchain,
        VkFramebuffer* pFramebuffers, VkCommandPool* commandPool,
        Buffers::VertexBuffer* vertexBuffer, Buffers::IndexBuffer* indexBuffer,
        VkDescriptorSet* descriptorSets, size_t objectCount, uint32_t dynamicAlignment
    );

    void cleanupSwapchain(
        VkDevice device,
        SwapchainData* pSwapchain,
        GraphicsPipelineData* pGraphicsPipeline,
        VkCommandPool commandPool,
        VkFramebuffer* pFramebuffers,
        VkCommandBuffer* pCommandbuffers,
        Buffers::BufferData* uniformBuffers,
        VkDescriptorPool& descriptorPool
    );

    VkResult createVertexBuffer(
        VulkanDeviceData* deviceData,
        Buffers::VertexBuffer*,
        VkCommandPool commandPool
    );

    VkResult createIndexBuffer(
        VulkanDeviceData* deviceData,
        Buffers::IndexBuffer* indexBuffer, 
        VkCommandPool commandPool
    );

    VkResult createUniformBuffers(
        VulkanDeviceData* deviceData,
        Buffers::BufferData* uBuffers,
        uint32_t imageCount
    );

    VkResult createDescriptorSets(
        VulkanDeviceData* deviceData,
        VkDescriptorSet* sets, VkDescriptorSetLayout* layout,
        VkDescriptorPool* pool, uint32_t imageCount,
        Buffers::BufferData* uBuffers, uint32_t bufferSize
    );

    VkCommandBuffer beginSingleTimeCommands(VkDevice, VkCommandPool);
    void endSingleTimeCommands(VkDevice, VkCommandBuffer&, VkQueue&, VkCommandPool);
    void copyBuffer(VkQueue, VkDevice, VkCommandPool, VkDeviceSize, VkBuffer, VkBuffer);
    void transitionImageLayout(VkDevice device, VkQueue queue, VkCommandPool commandPool, VkImage image, VkFormat format,
        VkImageLayout oldLayout, VkImageLayout newLayout);
    void copyBufferToImage(VkDevice, VkCommandPool, VkQueue, VkBuffer, VkImage, uint32_t, uint32_t);
}

#endif
