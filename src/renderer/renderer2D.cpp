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

        return true;
    }
}