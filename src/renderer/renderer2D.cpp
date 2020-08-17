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

    static uint32_t quadIndices[] = {
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

        return true;
    }
}