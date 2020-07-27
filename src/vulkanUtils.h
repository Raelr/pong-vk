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
    };

    SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device,
        VkSurfaceKHR surface);

    VkResult createSwapchain (
        SwapchainData* data,
        VulkanDeviceData* deviceData, 
        const uint32_t windowHeight,
        const uint32_t windowWidth
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
        VkBuffer vertexBuffer,
        uint32_t vertexCount
    );

    VkResult recreateSwapchain(
        VkDevice device,
        VkPhysicalDevice physicalDevice,
        SwapchainData* pSwapchain,
        VkSurfaceKHR surface,
        uint32_t window_width,
        uint32_t window_height,
        QueueFamilyIndices indices,
        GraphicsPipelineData* pGraphicsPipeline,
        VkCommandPool commandPool,
        VkFramebuffer* pFramebuffers,
        VkBuffer vertexBuffer,
        uint32_t vertexCount,
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
        VkDeviceMemory* vertexMemory, 
        const VertexBuffer::Vertex* vertices,
        const uint32_t vertexCount, 
        VkBuffer* vertexBuffer
    );

    bool findMemoryType(
        VkPhysicalDevice physicalDevice, 
        uint32_t* memoryType,
        uint32_t typeFilter, 
        VkMemoryPropertyFlags properties
    );
}

#endif
