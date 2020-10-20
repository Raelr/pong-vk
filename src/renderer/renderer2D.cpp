#include "renderer2D.h"
#include "vk/initialisers.h"
#include "vk/texture2d.h"


namespace Renderer2D {

    static Buffers::Vertex quadVertices[] = {
            // Positions        // Colors
            { {-0.5f,-0.5f } },
            { {0.5f, -0.5f } },
            { {0.5f,  0.5f } },
            { {-0.5f, 0.5f } }
    };

    static uint16_t quadIndices[] = {
        0, 1, 2, 2, 3, 0
    };

    bool initialiseRenderer2D(Renderer::VulkanDeviceData* deviceData,
        Renderer2DData* renderer2D, Renderer::SwapchainData swapchain, Renderer::Texture2D& texture) {

        // ================================== RENDER PASS ====================================

        if (Renderer::createRenderPass(deviceData->logicalDevice, swapchain.swapchainFormat,
            &renderer2D->graphicsPipeline) != VK_SUCCESS) {
            PONG_ERROR("Failed to create render pass!");
            return false;
        }

        // ============================== DESCRIPTOR SET LAYOUT ==============================

        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutBinding layoutBindings[] {
            Renderer::initiialiseDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_VERTEX_BIT),
            Renderer::initiialiseDescriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT)
        };

        if (Renderer::createDescriptorSetLayout(deviceData->logicalDevice,
            &renderer2D->quadData.descriptorSetLayout, layoutBindings, 2) != VK_SUCCESS) {
            PONG_ERROR("Failed to create descriptor set layout!");
            return false;
        }

        // ================================ GRAPHICS PIPELINE ================================

        // Vulkan requires that you define your own graphics pipelines when you
        // want to use different combinations of shaders. This is because the
        // graphics pipeline in Vulkan is almost completely immutable. This means
        // that the pipeline can be very well optimised (but will also require
        // a complete rewrite if you need anything different).

        if (Renderer::createGraphicsPipeline(deviceData->logicalDevice, &renderer2D->graphicsPipeline,
            &swapchain, &renderer2D->quadData.descriptorSetLayout) != VK_SUCCESS) {
            PONG_ERROR("Failed to create graphics pipeline!");
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

        if (Renderer::createFramebuffer(deviceData->logicalDevice, renderer2D->frameBuffers,
            &swapchain, &renderer2D->graphicsPipeline) != VK_SUCCESS) {
            PONG_ERROR("Failed to create framebuffers!");
            return false;
        }

        // ======================= VERTEX + INDEX BUFFER CREATION ========================

        renderer2D->quadData.vertexBuffer.vertices = quadVertices;
        renderer2D->quadData.vertexBuffer.vertexCount = 4;

        renderer2D->quadData.indexBuffer.indices = quadIndices;
        renderer2D->quadData.indexBuffer.indexCount = 6;

        // Create the vertex buffer and allocate the memory for it
        if (Renderer::createVertexBuffer(deviceData, &renderer2D->quadData.vertexBuffer,
            renderer2D->commandPool) != VK_SUCCESS) {
            PONG_ERROR("Failed to create vertex buffer.");
            return false;
        }

        // Create a uniform buffer for storing vertex data.
        if (Renderer::createIndexBuffer(deviceData, &renderer2D->quadData.indexBuffer,
            renderer2D->commandPool) != VK_SUCCESS) {
            PONG_ERROR("Failed to create index buffer.");
            return false;
        }

        VkDescriptorPoolSize poolSizes[] = {
                Renderer::initialisePoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, swapchain.imageCount),
                Renderer::initialisePoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, swapchain.imageCount)
        };

        if (Renderer::createDescriptorPool(
                deviceData->logicalDevice,
                swapchain.imageCount, &renderer2D->descriptorPool, poolSizes,
                2) != VK_SUCCESS) {

            PONG_ERROR("Failed to create descriptor pool.");
            return false;
        }

        // Calculate Uniform Buffer Memory
        Buffers::DynamicUniformBuffer<QuadProperties> dynamicUbo;
        Buffers::calculateBufferSize(&dynamicUbo, deviceData->physicalDevice, renderer2D->quadData.maxQuads);

        PONG_INFO("Buffer size: " + std::to_string(dynamicUbo.bufferSize));
        PONG_INFO("Dynamic alignment: " + std::to_string(dynamicUbo.dynamicAlignment));

        // CREATE UNIFORM BUFFER
        if (Buffers::createBuffer(
                deviceData->physicalDevice,
                deviceData->logicalDevice,
                dynamicUbo.bufferSize,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                dynamicUbo.buffer) != VK_SUCCESS) {

            return VK_ERROR_INITIALIZATION_FAILED;
        }

        renderer2D->quadData.dynamicData = dynamicUbo;

        VkDescriptorSet* descriptorSets =
                static_cast<VkDescriptorSet*>(malloc(swapchain.imageCount * sizeof(VkDescriptorSet)));

        if (Renderer::createDescriptorSets(
            deviceData,
            descriptorSets,
            &renderer2D->quadData.descriptorSetLayout,
            &renderer2D->descriptorPool,
            swapchain.imageCount,
            &renderer2D->quadData.dynamicData.buffer,
            renderer2D->quadData.dynamicData.bufferSize, texture) != VK_SUCCESS) {

            PONG_ERROR("Failed to create descriptor sets!");
            return false;
        }

        renderer2D->quadData.dynamicDescriptorSets = descriptorSets;

        // =============================== COMMAND BUFFERS ==================================

        renderer2D->commandBuffers = static_cast<VkCommandBuffer *>(malloc(
                swapchain.imageCount * sizeof(VkCommandBuffer)));

        // With the command pool created, we can now start creating and allocating
        // command buffers. Because these commands involve allocating a framebuffer,
        // we'll need to record a command buffer for every image in the swap chain.

        // It should be noted that command buffers are automatically cleaned up when
        // the commandpool is destroyed. As such they require no explicit cleanup.

        if (createCommandBuffers(
                deviceData->logicalDevice,
                renderer2D->commandBuffers,
                &renderer2D->graphicsPipeline,
                &swapchain,
                renderer2D->frameBuffers,
                renderer2D->commandPool,
                &renderer2D->quadData.vertexBuffer,
                &renderer2D->quadData.indexBuffer,
                renderer2D->quadData.dynamicDescriptorSets,
                renderer2D->quadData.quadCount,
                renderer2D->quadData.dynamicData.dynamicAlignment)
            != VK_SUCCESS) {

            PONG_ERROR("Failed to create command buffers!");
            return false;
        }

        return true;
    }

    void cleanupRenderer2D(Renderer::VulkanDeviceData* deviceData, Renderer2DData* pRenderer) {

        free(pRenderer->frameBuffers);
        free(pRenderer->quadData.dynamicDescriptorSets);

        vkDestroyDescriptorSetLayout(deviceData->logicalDevice,
                                     pRenderer->quadData.descriptorSetLayout,nullptr);

        // Cleans up the memory buffer
        vkDestroyBuffer(deviceData->logicalDevice,
                        pRenderer->quadData.vertexBuffer.bufferData.buffer, nullptr);

        // Frees the allocated vertex buffer memory
        vkFreeMemory(deviceData->logicalDevice,
                     pRenderer->quadData.vertexBuffer.bufferData.bufferMemory, nullptr);

        vkDestroyBuffer(deviceData->logicalDevice,
                        pRenderer->quadData.indexBuffer.bufferData.buffer, nullptr);

        vkFreeMemory(deviceData->logicalDevice,
                     pRenderer->quadData.indexBuffer.bufferData.bufferMemory, nullptr);

        Buffers::alignedFree(pRenderer->quadData.dynamicData.data);
        free(pRenderer->commandBuffers);
    }

    bool recreateRenderer2D(Renderer::VulkanDeviceData* deviceData, Renderer2DData* renderer2D,
        Renderer::SwapchainData swapchain) {

        // ================================== RENDER PASS ====================================

        if (Renderer::createRenderPass(deviceData->logicalDevice, swapchain.swapchainFormat,
                                       &renderer2D->graphicsPipeline) != VK_SUCCESS) {
            PONG_ERROR("Failed to create render pass!");
            return false;
        }

        // ================================ GRAPHICS PIPELINE ================================

        if (Renderer::createGraphicsPipeline(deviceData->logicalDevice, &renderer2D->graphicsPipeline,
                                             &swapchain, &renderer2D->quadData.descriptorSetLayout) != VK_SUCCESS) {
            PONG_ERROR("Failed to create graphics pipeline!");
            return false;
        }

        // ================================ FRAMEBUFFER SETUP ================================

        if (Renderer::createFramebuffer(deviceData->logicalDevice, renderer2D->frameBuffers,
                                        &swapchain, &renderer2D->graphicsPipeline) != VK_SUCCESS) {
            PONG_ERROR("Failed to create framebuffers!");
            return false;
        }

        VkDescriptorPoolSize poolSizes[] = {
                Renderer::initialisePoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, swapchain.imageCount)
        };

        if (Renderer::createDescriptorPool(
                deviceData->logicalDevice,
                swapchain.imageCount, &renderer2D->descriptorPool, poolSizes,
                1) != VK_SUCCESS) {

            PONG_ERROR("Failed to create descriptor pool.");
            return false;
        }

        Buffers::DynamicUniformBuffer<Renderer2D::QuadProperties> dynamicUbo;
        Buffers::calculateBufferSize(&dynamicUbo, deviceData->physicalDevice, renderer2D->quadData.maxQuads);

        PONG_INFO("Buffer size: " + std::to_string(dynamicUbo.bufferSize));
        PONG_INFO("Dynamic alignment: " + std::to_string(dynamicUbo.dynamicAlignment));

        // ALLOCATE UNIFORM BUFFER
        if (Buffers::createBuffer(
                deviceData->physicalDevice,
                deviceData->logicalDevice,
                dynamicUbo.bufferSize,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                dynamicUbo.buffer) != VK_SUCCESS) {

            return VK_ERROR_INITIALIZATION_FAILED;
        }

        renderer2D->quadData.dynamicData = dynamicUbo;

        if (Renderer::createDescriptorSets(
                deviceData,
                renderer2D->quadData.dynamicDescriptorSets,
                &renderer2D->quadData.descriptorSetLayout,
                &renderer2D->descriptorPool,
                swapchain.imageCount,
                &renderer2D->quadData.dynamicData.buffer,
                renderer2D->quadData.dynamicData.bufferSize) != VK_SUCCESS) {

            PONG_ERROR("Failed to create descriptor sets!");
            return false;
        }

        return true;
    }
}