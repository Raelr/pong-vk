#include "renderer2D.h"
#include "../logger.h"

namespace Renderer2D {

    static Buffers::Vertex quadVertices[] = {
            // Positions        // Colors
            { {-0.5f, -0.5f},   {1.0f, 1.0f, 1.0f} },
            { {0.5f, -0.5f},    {1.0f, 1.0f, 1.0f} },
            { {0.5f, 0.5f},     {1.0f, 1.0f, 1.0f} },
            { {-0.5f, 0.5f},    {1.0f, 1.0f, 1.0f} }
    };

    static uint16_t quadIndices[] = {
        0, 1, 2, 2, 3, 0
    };

    bool initialiseRenderer2D(VulkanUtils::VulkanDeviceData* deviceData,
        Renderer2DData* renderer2D, VulkanUtils::SwapchainData swapchain) {

        // ================================== RENDER PASS ====================================

        if (VulkanUtils::createRenderPass(deviceData->logicalDevice, swapchain.swapchainFormat,
            &renderer2D->graphicsPipeline) != VK_SUCCESS) {
            ERROR("Failed to create render pass!");
            return false;
        }

        // ============================== DESCRIPTOR SET LAYOUT ==============================

        if (VulkanUtils::createDescriptorSetLayout(deviceData->logicalDevice,
            &renderer2D->quadData.descriptorSetLayout) != VK_SUCCESS) {
            ERROR("Failed to create descriptor set layout!");
            return false;
        }

        // ================================ GRAPHICS PIPELINE ================================

        // Vulkan requires that you define your own graphics pipelines when you
        // want to use different combinations of shaders. This is because the
        // graphics pipeline in Vulkan is almost completely immutable. This means
        // that the pipeline can be very well optimised (but will also require
        // a complete rewrite if you need anything different).

        if (VulkanUtils::createGraphicsPipeline(deviceData->logicalDevice, &renderer2D->graphicsPipeline,
            &swapchain, &renderer2D->quadData.descriptorSetLayout) != VK_SUCCESS) {
            ERROR("Failed to create graphics pipeline!");
            return false;
        }

        // ================================ FRAMEBUFFER SETUP ================================

        // At this point, we now need to create the framebuffers which will be
        // used for our renderpass. These framebuffers should be used in the
        // same format as our swapchain images (which we defined before).

        // We use a VkFramebuffer object to store the attachments which were specified
        // during the render pass. A framebuffer references all the image views that
        // represent these attachments. In our case, we only have one to reference,
        // namely the color attachment.

        renderer2D->frameBuffers = static_cast<VkFramebuffer*>(malloc(swapchain.imageCount * sizeof(VkFramebuffer)));

        if (VulkanUtils::createFramebuffer(deviceData->logicalDevice, renderer2D->frameBuffers,
            &swapchain, &renderer2D->graphicsPipeline) != VK_SUCCESS) {
            ERROR("Failed to create framebuffers!");
            return false;
        }

        // ========================= COMMAND POOL CREATION =========================

        // In vulkan, all steps and operations that happen are not handled via
        // function calls. Rather, these steps are recorded in a CommandBuffer
        // object which are then executed later in runtime. As such, commands
        // allow us to set everything up in advance and in multiple threads if
        // need be. These commands are then handled by Vulkan in the main loop.

        // Command buffers are generally executed by submitting them to one of the
        // device queues (such as the graphics and presentation queues we set earlier).
        // Command pools can only submit buffers to a single type of queue. Since
        // we're going to be submitting data for drawing, we'll use the graphics
        // queue.

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = deviceData->indices.graphicsFamily.value();
        poolInfo.flags = 0; // optional

        // Create the command pool
        if (vkCreateCommandPool(deviceData->logicalDevice, &poolInfo, nullptr,
            &renderer2D->commandPool) != VK_SUCCESS) {
            ERROR("Failed to create command pool!");
            return false;
        }

        // ==================== VERTEX + INDEX BUFFER CREATION ===========================

        renderer2D->quadData.vertexBuffer.vertices = quadVertices;
        renderer2D->quadData.vertexBuffer.vertexCount = 4;

        renderer2D->quadData.indexBuffer.indices = quadIndices;
        renderer2D->quadData.indexBuffer.indexCount = 6;

        // Create the vertex buffer and allocate the memory for it
        if (VulkanUtils::createVertexBuffer(deviceData, &renderer2D->quadData.vertexBuffer,
            renderer2D->commandPool) != VK_SUCCESS) {
            ERROR("Failed to create vertex buffer.");
            return false;
        }

        // Create a uniform buffer for storing vertex data.
        if (VulkanUtils::createIndexBuffer(deviceData, &renderer2D->quadData.indexBuffer,
            renderer2D->commandPool) != VK_SUCCESS) {
            ERROR("Failed to create index buffer.");
            return false;
        }


        return true;
    }
}