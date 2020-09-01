#ifndef VULKAN_UTILS_H
#define VULKAN_UTILS_H

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>
#include "../utils.h"
#include "buffers.h"

namespace VulkanUtils {

    // Struct for holding all swapchain relevant data.
    struct SwapchainDataTemp {
        VkSwapchainKHR swapchain;
        uint32_t imageCount;
        VkFormat swapchainFormat;
        VkExtent2D swapchainExtent;
        VkImageView* pImageViews;
        VkImage* pImages;
    };
    // Struct storing details relating to swapchain extensions and
    // support.
    struct SwapchainSupportDetailsTemp {
        VkSurfaceCapabilitiesKHR capabilities;
        VkSurfaceFormatKHR* formats;
        uint32_t formatCount;
        VkPresentModeKHR* presentModes;
        uint32_t presentModesCount;
    };
    // Struct storing all queue families
    struct QueueFamilyIndicesTemp {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
    };

    struct GraphicsPipelineData {
        VkRenderPass renderPass;
        VkPipeline graphicsPipeline;
        VkPipelineLayout pipelineLayout;
    };

    struct VulkanDeviceDataTemp {
        VkPhysicalDevice physicalDevice;
        VkDevice logicalDevice;
        VkSurfaceKHR surface;
        QueueFamilyIndicesTemp indices;
        int framebufferWidth;
        int framebufferHeight;
        VkQueue graphicsQueue;
        VkQueue presentQueue;
    };

    void cleanupSwapchainSupportDetails(SwapchainSupportDetailsTemp*);

    SwapchainSupportDetailsTemp querySwapchainSupport(VkPhysicalDevice device,
        VkSurfaceKHR surface);

    VkResult createSwapchain (
        SwapchainDataTemp* data,
        VulkanDeviceDataTemp* deviceData
    );

    QueueFamilyIndicesTemp findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR
        surface);
    
    void destroySwapchainImageData(SwapchainDataTemp);

    VkResult createImageViews(VkDevice, SwapchainDataTemp*);

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
        const SwapchainDataTemp*,
        VkDescriptorSetLayout* descriptorSetLayout
    );

    VkShaderModule createShaderModule(
        FileContents buffer, 
        VkDevice &device 
    );

    VkResult createFramebuffer(
        VkDevice device, 
        VkFramebuffer* pFramebuffers,
        SwapchainDataTemp* swapchain,
        GraphicsPipelineData* graphicsPipeline
    );

    VkResult createCommandBuffers(
        VkDevice device,
        VkCommandBuffer* buffers,
        GraphicsPipelineData* pGraphicsPipeline,
        SwapchainDataTemp* pSwapchain,
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
            SwapchainDataTemp* pSwapchain,
            VkFramebuffer* pFramebuffers,
            VkCommandPool* commandPool,
            Buffers::VertexBuffer*,
            Buffers::IndexBuffer*,
            VkDescriptorSet** descriptorSets,
            size_t objectCount
    );

    VkResult recreateSwapchain(
        VulkanDeviceDataTemp* deviceData,
        SwapchainDataTemp* pSwapchain,
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
        SwapchainDataTemp* pSwapchain,
        GraphicsPipelineData* pGraphicsPipeline,
        VkCommandPool commandPool,
        VkFramebuffer* pFramebuffers,
        VkCommandBuffer* pCommandbuffers,
        Buffers::BufferData** uniformBuffers,
        VkDescriptorPool& descriptorPool,
        size_t objectCount
    );

    VkResult createVertexBuffer(
        VulkanDeviceDataTemp* deviceData,
        Buffers::VertexBuffer*,
        VkCommandPool commandPool
    );

    VkResult createIndexBuffer(
        VulkanDeviceDataTemp* deviceData,
        Buffers::IndexBuffer* indexBuffer, 
        VkCommandPool commandPool
    );

    VkResult createUniformBuffers(
        VulkanDeviceDataTemp* deviceData,
        Buffers::BufferData* uBuffers,
        uint32_t imageCount
    );

    VkResult createDescriptorSets(
        VulkanDeviceDataTemp* deviceData,
        VkDescriptorSet* sets,
        VkDescriptorSetLayout* layout,
        VkDescriptorPool* pool,
        uint32_t imageCount,
        Buffers::BufferData* uBuffers
    );
}

#endif
