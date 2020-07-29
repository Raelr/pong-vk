#ifndef VULKAN_UTILS_H
#define VULKAN_UTILS_H

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>
#include "utils.h"
#include "vertexBuffer.h"

namespace VulkanUtils {

    // Struct for holding all swapchain relevant data.
    struct SwapchainData {
        VkSwapchainKHR swapchain;
        uint32_t imageCount;
        VkFormat swapchainFormat;
        VkExtent2D swapchainExtent;
        VkImageView* pImageViews;
        VkImage* pImages;
    };
    // Struct storing details relating to swapchain extensions and
    // support.
    struct SwapchainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };
    // Struct storing all queue families
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
    };

    struct GraphicsPipelineData {
        VkRenderPass renderPass;
        VkPipeline graphicsPipeline;
        VkPipelineLayout pipelineLayout;
    };

    struct VulkanDeviceData {
        VkPhysicalDevice physicalDevice;
        VkDevice logicalDevice;
        VkSurfaceKHR surface;
        QueueFamilyIndices indices;
        int framebufferWidth;
        int framebufferHeight;
    };

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

    VkResult createGraphicsPipeline(
        VkDevice, 
        GraphicsPipelineData*, 
        const SwapchainData*
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
        VertexBuffer::VertexBuffer*
    );

    VkResult recreateSwapchain(
        VulkanDeviceData* deviceData,
        SwapchainData* pSwapchain,
        GraphicsPipelineData* pGraphicsPipeline,
        VkCommandPool commandPool,
        VkFramebuffer* pFramebuffers,
        VertexBuffer::VertexBuffer* vertexBuffer,
        VkCommandBuffer* pCommandbuffers
    );

    void cleanupSwapchain(
        VkDevice device,
        SwapchainData* pSwapchain,
        GraphicsPipelineData* pGraphicsPipeline,
        VkCommandPool commandPool,
        VkFramebuffer* pFramebuffers,
        VkCommandBuffer* pCommandbuffers
    );

    VkResult createVertexBuffer(
        VulkanDeviceData* deviceData,
        VertexBuffer::VertexBuffer*
    );

    VkResult createBuffer(
        VulkanDeviceData* deviceData,
        VertexBuffer::VertexBuffer* vertexBuffer, 
        VkDeviceSize size,
        VkBufferUsageFlags usage, 
        VkMemoryPropertyFlags properties
    );

    bool findMemoryType(
        VkPhysicalDevice physicalDevice, 
        uint32_t* memoryType,
        uint32_t typeFilter, 
        VkMemoryPropertyFlags properties
    );
}

#endif
