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

    void cleanupSwapchainSupportDetails(SwapchainSupportDetails*);

    SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device,
        VkSurfaceKHR surface);

    VkResult createSwapchain (
        SwapchainData* data,
        VulkanDeviceData* deviceData
    );

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR
        surface);
    
    void destroySwapchainImageData(SwapchainData);

    VkResult createImageViews(VkDevice, SwapchainData*);

    VkResult createRenderPass(VkDevice, VkFormat format, GraphicsPipelineData*);

    VkResult createDescriptorSetLayout(
        VkDevice device,
        VkDescriptorSetLayout* descriptorSetLayout
    );

    VkResult createDescriptorPool(
        VkDevice device,
        uint32_t imageCount,
        VkDescriptorPool* descriptorPool,
        size_t objectCount
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
        VkDescriptorSet** descriptorSets,
        size_t objectCount
    );

    VkResult createCommandBuffer(
            VkDevice device,
            VkCommandBuffer* buffers,
            size_t bufferIndex,
            GraphicsPipelineData* pGraphicsPipeline,
            SwapchainData* pSwapchain,
            VkFramebuffer* pFramebuffers,
            VkCommandPool* commandPool,
            Buffers::VertexBuffer*,
            Buffers::IndexBuffer*,
            VkDescriptorSet** descriptorSets,
            size_t objectCount
    );

    VkResult recreateSwapchain(
        VulkanDeviceData* deviceData,
        SwapchainData* pSwapchain,
        GraphicsPipelineData* pGraphicsPipeline,
        VkCommandPool commandPool,
        VkFramebuffer* pFramebuffers,
        Buffers::VertexBuffer* vertexBuffer,
        Buffers::IndexBuffer* indexBuffer,
        VkCommandBuffer* pCommandbuffers,
        VkDescriptorSetLayout* descriptorSetLayout,
        VkDescriptorPool* descriptorPool,
        VkDescriptorSet** descriptorSets,
        Buffers::BufferData** uniformBuffers,
        size_t objectCount
    );

    void cleanupSwapchain(
        VkDevice device,
        SwapchainData* pSwapchain,
        GraphicsPipelineData* pGraphicsPipeline,
        VkCommandPool commandPool,
        VkFramebuffer* pFramebuffers,
        VkCommandBuffer* pCommandbuffers,
        Buffers::BufferData** uniformBuffers,
        VkDescriptorPool& descriptorPool,
        size_t objectCount
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
        VkDescriptorSet* sets,
        VkDescriptorSetLayout* layout,
        VkDescriptorPool* pool,
        uint32_t imageCount,
        Buffers::BufferData* uBuffers
    );
}

#endif
