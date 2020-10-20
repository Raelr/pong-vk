#include "renderer.h"
#include <cstring>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include "vk/initialisers.h"
#include <glm/gtx/string_cast.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "vk/texture2d.h"

namespace Renderer {

    static const char* validationLayers[] = {
        "VK_LAYER_KHRONOS_validation"
    };

    static const char* deviceExtensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    Status initialiseRenderer(Renderer* renderer, bool enableValidationLayers, void* nativeWindow, WindowType type) {

        if (type == WindowType::GLFW) {
            auto window = static_cast<GLFWwindow*>(nativeWindow);

            // Initialise Vulkan device information - this should be pretty general so won't require too
            // much configuration.
            if (createVulkanDeviceData(&renderer->deviceData, window, enableValidationLayers)
                != Status::SUCCESS) {
                PONG_ERROR("Failed to create Vulkan Device. Closing Pong...");
                return Status::FAILURE;
            }

            glfwGetFramebufferSize(window, &renderer->deviceData.framebufferWidth,
                &renderer->deviceData.framebufferHeight);
        }

        // ============================= SWAPCHAIN CREATION =================================

        // Create the swapchain (should initialise both the swapchain and image views)
        if (createSwapchain(&renderer->swapchainData, &renderer->deviceData) != VK_SUCCESS) {
            PONG_ERROR("Failed to create swapchain!");
            return Status::FAILURE;
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
        poolInfo.queueFamilyIndex = renderer->deviceData.indices.graphicsFamily.value();
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // optional

        // Create the command pool
        if (vkCreateCommandPool(renderer->deviceData.logicalDevice, &poolInfo, nullptr,
                                &renderer->renderer2DData.commandPool) != VK_SUCCESS) {
            PONG_ERROR("Failed to create command pool!");
            return Status::INITIALIZATION_FAILURE;
        }

        PONG_INFO("Initialised Swapchain");

        Texture2D texture;
        loadImage(renderer, "assets/awesomeface.png", texture);
        texture.sampler = initialiseSampler(
                renderer->deviceData.logicalDevice,
                VK_FILTER_LINEAR, VK_FILTER_LINEAR,
                VK_SAMPLER_ADDRESS_MODE_REPEAT,
                VK_BORDER_COLOR_INT_OPAQUE_BLACK,
                VK_COMPARE_OP_ALWAYS,
                VK_SAMPLER_MIPMAP_MODE_LINEAR
        );

        // ================================= RENDERER 2D ====================================

        if (!Renderer2D::initialiseRenderer2D(&renderer->deviceData,
            &renderer->renderer2DData, renderer->swapchainData, texture)) {
            PONG_ERROR("Failed to create renderer2D");
            return Status::INITIALIZATION_FAILURE;
        }

        PONG_INFO("Initialised renderer2D!");


        // ================================ SYNC OBJECTS ====================================

        createSyncObjects(renderer);

        PONG_INFO("Created synchronisation objects");

        return Status::SUCCESS;
    }

    void loadDefaultValidationLayers(Renderer* renderer) {

        renderer->deviceData.validationLayers = validationLayers;
        renderer->deviceData.validationLayerCount = 1;
    }

    [[maybe_unused]] Status loadCustomValidationLayers(Renderer* renderer,
                        const char** pValidationLayers, uint32_t pValidationLayersCount) {

        Status status = Status::FAILURE;
        if (pValidationLayers != nullptr && pValidationLayersCount != 0) {
            renderer->deviceData.validationLayers = pValidationLayers;
            renderer->deviceData.validationLayerCount = pValidationLayersCount;
            status = Status::SUCCESS;
        } else {
            PONG_ERROR("Unable to load in validation layers! Invalid validation layers have been provided");
        }

        return status;
    }

    void loadDefaultDeviceExtensions(Renderer* renderer) {

        renderer->deviceData.deviceExtensions = deviceExtensions;
        renderer->deviceData.deviceExtensionCount = 1;
    }

    [[maybe_unused]] Status loadCustomDeviceExtensions(Renderer* renderer, const char** extensions,
                       uint32_t extensionCount) {

        Status status = Status::FAILURE;

        if (extensions != nullptr && extensionCount > 0) {
            renderer->deviceData.deviceExtensions = extensions;
            renderer->deviceData.deviceExtensionCount = extensionCount;
            status = Status::SUCCESS;
        }

        return status;
    }

    Status cleanupRenderer(Renderer* pRenderer, bool enableValidationLayers) {

        // Since our image drawing is asynchronous, we need to make sure that
        // our resources aren't in use when trying to clean them up:
        vkDeviceWaitIdle(pRenderer->deviceData.logicalDevice);

        cleanupSwapchain(
            pRenderer->deviceData.logicalDevice,
            &pRenderer->swapchainData,
            &pRenderer->renderer2DData.graphicsPipeline,
            pRenderer->renderer2DData.commandPool,
            pRenderer->renderer2DData.frameBuffers,
            pRenderer->renderer2DData.commandBuffers,
            &pRenderer->renderer2DData.quadData.dynamicData.buffer,
            pRenderer->renderer2DData.descriptorPool
        );

        Renderer2D::cleanupRenderer2D(&pRenderer->deviceData, &pRenderer->renderer2DData);

        // Clean up the semaphores we created earlier.
        for (size_t i = 0; i < pRenderer->maxFramesInFlight; i++) {
            vkDestroySemaphore(pRenderer->deviceData.logicalDevice, pRenderer->renderFinishedSemaphores[i],
        nullptr);
            vkDestroySemaphore(pRenderer->deviceData.logicalDevice, pRenderer->imageAvailableSemaphores[i],
        nullptr);
            vkDestroyFence(pRenderer->deviceData.logicalDevice, pRenderer->inFlightFences[i], nullptr);
        }

        // Free the memory used by the arrays
        free(pRenderer->imageAvailableSemaphores);
        free(pRenderer->inFlightFences);
        free(pRenderer->renderFinishedSemaphores);

        vkDestroyCommandPool(pRenderer->deviceData.logicalDevice, pRenderer->renderer2DData.commandPool,
    nullptr);

        cleanupVulkanDevice(&pRenderer->deviceData, enableValidationLayers);

        return Status::SUCCESS;
    }

    Status createSyncObjects(Renderer* pRenderer, uint32_t maxFramesInFlight) {

        // In Vulkan, drawing every frame can be done in a few simple function
        // calls. These function calls are executed asynchronously - they are
        // executed and returned in no particular order. As such, we aren't always
        // guaranteed to get the images we need to execute the next frame. This
        // would then lead to serious memory violations.

        // As such, Vulkan requires us to synchronise our swap chain events, namely
        // through the use of semaphores and fences. In both cases, our program will
        // be forced to stop execution until a certain resource is made available
        // to it (when the semaphore moves from 'signalled' to 'un-signalled').

        // The main difference between semaphores and fences is that fences can be
        // accessed by our program through a number of function calls. Semaphores,
        // on the other hand, cannot. Semaphores are primarily used to synchronise
        // operations within or across command queues.

        // For now, we'll be using semaphores to get a triangle rendering!

        // Create two semaphores - one which signals when an image is ready to be
        // used, another for when the render pass has finished.

        if (maxFramesInFlight > 0) {
            pRenderer->maxFramesInFlight = maxFramesInFlight;
        }

        pRenderer->imageAvailableSemaphores = static_cast<VkSemaphore *>(malloc(
        pRenderer->maxFramesInFlight * sizeof(VkSemaphore)));

        pRenderer->renderFinishedSemaphores = static_cast<VkSemaphore *>(malloc(
                pRenderer->maxFramesInFlight * sizeof(VkSemaphore)));

        // The semaphore info struct only has one field
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        pRenderer->inFlightFences = static_cast<VkFence *>(malloc(
                pRenderer->maxFramesInFlight * sizeof(VkFence)));

        pRenderer->imagesInFlight = static_cast<VkFence *>(malloc(
                pRenderer->swapchainData.imageCount * sizeof(VkFence)));

        // Initialise all these images to 0 to start with.
        for (size_t i = 0; i < pRenderer->swapchainData.imageCount; i++) {
            pRenderer->imagesInFlight[i] = VK_NULL_HANDLE;
        }

        // As always with Vulkan, we create a create info struct to handle the
        // configuration.
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        // Specify that the fence should be started in a signalled state.
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        // Now we simply create both semaphores, making sure that both succeed before we
        // move on.
        for (size_t i = 0; i < pRenderer->maxFramesInFlight; i++) {
            if (vkCreateSemaphore(pRenderer->deviceData.logicalDevice, &semaphoreInfo, nullptr,
                    &pRenderer->imageAvailableSemaphores[i]) != VK_SUCCESS
                ||
                vkCreateSemaphore(pRenderer->deviceData.logicalDevice, &semaphoreInfo, nullptr,
                    &pRenderer->renderFinishedSemaphores[i]) != VK_SUCCESS
                ||
                vkCreateFence(pRenderer->deviceData.logicalDevice, &fenceInfo, nullptr,
                    &pRenderer->inFlightFences[i]) != VK_SUCCESS) {

                PONG_ERROR("Failed to create synchronisation objects for frame!");
                return Status::INITIALIZATION_FAILURE;
            }
        }

        return Status::SUCCESS;
    }

    Status drawFrame(Renderer* pRenderer, bool* resized) {

        // This function takes an array of fences and waits for either one or all
        // of them to be signalled. The fourth parameter specifies that we're
        // waiting for all fences to be signalled before moving on. The last
        // parameter takes a timeout period which we set really high (effectively
        // making it null)
        vkWaitForFences(pRenderer->deviceData.logicalDevice, 1,
            &pRenderer->inFlightFences[pRenderer->currentFrame], VK_TRUE, UINT64_MAX);

        // In each frame of the main loop, we'll need to perform the following
        // operations:
        // 1. acquire an image from the swapchain.
        // 2. execute the command buffer with that image as an attachment.
        // 3. Return the image to the swapchain for presentation.

        // First we need to acquire an image from the swapchain. For this we need
        // our logical device and swapchain. The third parameter is a timeout period
        // which we disable using the max of a 64-bit integer. Next we provide our
        // semaphore, and finally a variable to output the image index to.
        VkResult result = vkAcquireNextImageKHR(pRenderer->deviceData.logicalDevice,
            pRenderer->swapchainData.swapchain, UINT64_MAX, pRenderer->imageAvailableSemaphores[pRenderer->currentFrame],
            VK_NULL_HANDLE, &pRenderer->imageIndex);

        if (vkGetFenceStatus(pRenderer->deviceData.logicalDevice, pRenderer->inFlightFences[pRenderer->currentFrame])
            == VK_SUCCESS) {

            vkResetCommandBuffer(pRenderer->renderer2DData.commandBuffers[pRenderer->imageIndex],
            VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

            if (rerecordCommandBuffer(
                    pRenderer->deviceData.logicalDevice,
                    &pRenderer->renderer2DData.commandBuffers[pRenderer->imageIndex],
                    pRenderer->imageIndex,
                    &pRenderer->renderer2DData.graphicsPipeline,
                    &pRenderer->swapchainData,
                    pRenderer->renderer2DData.frameBuffers,
                    &pRenderer->renderer2DData.commandPool,
                    &pRenderer->renderer2DData.quadData.vertexBuffer,
                    &pRenderer->renderer2DData.quadData.indexBuffer,
                    pRenderer->renderer2DData.quadData.dynamicDescriptorSets,
                    pRenderer->renderer2DData.quadData.quadCount,
                    pRenderer->renderer2DData.quadData.dynamicData.dynamicAlignment) != VK_SUCCESS) {

                PONG_ERROR("Failed to re-record command buffer!");
                return Status::FAILURE;
            }
        }

        // If our swapchain is out of date (no longer valid, then we re-create
        // it)
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            // Go to the next iteration of the loop
            return Status::SKIPPED_FRAME;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            PONG_ERROR("Failed to acquire swapchain image!");
            return Status::FAILURE;
        }

        // Check if a previous frame is using this image. I.e: we're waiting on
        // it's fence to be signalled.
        if (pRenderer->imagesInFlight[pRenderer->imageIndex] != VK_NULL_HANDLE) {
            // Wait for the fence to signal that it's available for usage. This
            // will now ensure that there are no more than 2 frames in use, and
            // that these frames are not accidentally using the same image!
            vkWaitForFences(pRenderer->deviceData.logicalDevice, 1,
                &pRenderer->imagesInFlight[pRenderer->imageIndex], VK_TRUE, UINT64_MAX);
        }
        // Now, use the image in this frame!.
        pRenderer->imagesInFlight[pRenderer->imageIndex] = pRenderer->inFlightFences[pRenderer->currentFrame];

        // Once we have that, we now need to submit the image to the queue:
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        // We need to specify which semaphores to wait on before executing the frame.
        VkSemaphore waitSemaphores[] = { pRenderer->imageAvailableSemaphores[pRenderer->currentFrame] };
        // We also need to specify which stages of the pipeline need to be done so we can move
        // on.
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        // A count of all semaphores.
        submitInfo.waitSemaphoreCount = 1;
        // Finaly, input the semaphores and stages.
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        // Now we need to specify which command buffers to submit to. In our
        // case we need to submit to the buffer which corresponds to our image.
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &pRenderer->renderer2DData.commandBuffers[pRenderer->imageIndex];
        // Now we specify which semaphores we need to signal once our command buffers
        // have finished execution.
        VkSemaphore signalSemaphores[] = { pRenderer->renderFinishedSemaphores[pRenderer->currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        // We need to reset the fence to an unsignalled state before moving on.
        vkResetFences(pRenderer->deviceData.logicalDevice, 1, &pRenderer->inFlightFences[pRenderer->currentFrame]);

        // Finally, we submit the buffer to the graphics queue
        if (vkQueueSubmit(pRenderer->deviceData.graphicsQueue, 1, &submitInfo, pRenderer->inFlightFences[pRenderer->currentFrame])
            != VK_SUCCESS) {
            PONG_ERROR("Failed to submit draw command buffer!");
            return Status::FAILURE;
        }

        // The final step to drawing a frame is resubmitting the the result back
        // to the swapchain. This is done by configuring our swapchain presentation.

        // Start with a PresentInfo struct:
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        // First we specify how many semaphores we need to wait on.
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        // Next we need to specify which swapchains we're using:
        VkSwapchainKHR swapchains[] = { pRenderer->swapchainData.swapchain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapchains;
        // Get the index of the image we're using:
        presentInfo.pImageIndices = &pRenderer->imageIndex;
        // Allows you to get an array of VK_RESULTs which tell you if presentation
        // of every swapchain was successful.
        presentInfo.pResults = nullptr; // optional

        // Now we submit a request to present an image to the swapchain.
        result = vkQueuePresentKHR(pRenderer->deviceData.presentQueue, &presentInfo);

        // Again, we make sure that we're using the best possible swapchain.
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR
            || *resized) {

            return Status::SKIPPED_FRAME;
        } else if (result != VK_SUCCESS) {
            PONG_ERROR("Failed to present swapchain image!");
            return Status::FAILURE;
        }

        // This should clamp the value of currentFrame between 0 and 1.
        pRenderer->currentFrame = (pRenderer->currentFrame + 1) % pRenderer->maxFramesInFlight;

        return Status::SUCCESS;
    }

    // Handles a case where the window is minimised - pauses rendering until its opened again.
    Status onWindowMinimised(GLFWwindow* window, int* width, int* height) {

        glfwGetFramebufferSize(window, width, height);

        // If the window is minimized we simply pause rendering until it
        // comes back!

        while (*width == 0 && *height == 0) {
            glfwGetFramebufferSize(window, width, height);
            glfwWaitEvents();
        }

        return Status::SUCCESS;
    }

    Status drawQuad(Renderer* pRenderer, glm::vec3 pos, glm::vec3 rot, float degrees, glm::vec3 scale, glm::vec3 color) {

        Buffers::DynamicUniformBuffer<Renderer2D::QuadProperties>& dynamicUniformBuffer = pRenderer->renderer2DData.quadData.dynamicData;

        glm::mat4 model = glm::mat4(1.0f);

        model = glm::translate(model, pos);
        model = glm::rotate(model, degrees, rot);
        model = glm::scale(model, scale);

        // Set the view
        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -1.0f));

        // TODO: Need to find a way to pass the original size to the ortho camera to keep it stretching
        glm::mat4 proj = glm::ortho(-400.0f, 400.f, 300.0f, -300.0f, -1.0f, 1.0f);

        // Rotate the model matrix
        model = proj * view * model;

        auto* properties = (Renderer2D::QuadProperties*)(((uint64_t)dynamicUniformBuffer.data
                + (pRenderer->renderer2DData.quadData.quadCount * dynamicUniformBuffer.dynamicAlignment)));

        properties->mvp = model;
        properties->color = color;

        VkDeviceMemory memory = pRenderer->renderer2DData.quadData.dynamicData.buffer.bufferMemory;

        // Now we bind our data to the UBO for later use
        void* data;
        vkMapMemory(pRenderer->deviceData.logicalDevice, memory,0, VK_WHOLE_SIZE, 0, &data);
        memcpy(data, pRenderer->renderer2DData.quadData.dynamicData.data, pRenderer->renderer2DData.quadData.dynamicData.bufferSize);

        VkMappedMemoryRange mappedMemoryRange {};
        mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedMemoryRange.memory = pRenderer->renderer2DData.quadData.dynamicData.buffer.bufferMemory;
        mappedMemoryRange.size = pRenderer->renderer2DData.quadData.dynamicData.bufferSize;
        vkFlushMappedMemoryRanges(pRenderer->deviceData.logicalDevice, 1, &mappedMemoryRange);

        vkUnmapMemory(pRenderer->deviceData.logicalDevice, memory);

        pRenderer->renderer2DData.quadData.quadCount++;

        return Status::SUCCESS;
    }

    VkResult recreateSwapchain(Renderer* pRenderer) {

        vkDeviceWaitIdle(pRenderer->deviceData.logicalDevice);

        cleanupSwapchain(
            pRenderer->deviceData.logicalDevice, &pRenderer->swapchainData,
            &pRenderer->renderer2DData.graphicsPipeline,
            pRenderer->renderer2DData.commandPool, pRenderer->renderer2DData.frameBuffers,
            pRenderer->renderer2DData.commandBuffers, &pRenderer->renderer2DData.quadData.dynamicData.buffer,
            pRenderer->renderer2DData.descriptorPool
        );

        // Re-populate the swapchain
        if (createSwapchain(&pRenderer->swapchainData, &pRenderer->deviceData) != VK_SUCCESS) {

            return VK_ERROR_INITIALIZATION_FAILED;
        }

        if (!Renderer2D::recreateRenderer2D(&pRenderer->deviceData, &pRenderer->renderer2DData, pRenderer->swapchainData)) {
            PONG_ERROR("Failed to re-create swap chain on resize!");
            return VK_ERROR_INITIALIZATION_FAILED;
        }

        if (createCommandBuffers(
                pRenderer->deviceData.logicalDevice,
                pRenderer->renderer2DData.commandBuffers,
                &pRenderer->renderer2DData.graphicsPipeline,
                &pRenderer->swapchainData,
                pRenderer->renderer2DData.frameBuffers,
                pRenderer->renderer2DData.commandPool,
                &pRenderer->renderer2DData.quadData.vertexBuffer,
                &pRenderer->renderer2DData.quadData.indexBuffer,
                pRenderer->renderer2DData.quadData.dynamicDescriptorSets,
                pRenderer->renderer2DData.quadData.quadCount,
                pRenderer->renderer2DData.quadData.dynamicData.dynamicAlignment)
            != VK_SUCCESS) {

            PONG_ERROR("Failed to create command buffers!");
            return VK_ERROR_INITIALIZATION_FAILED;
        }

        return VK_SUCCESS;
    }

    void flushRenderer(Renderer* pRenderer) {
        pRenderer->renderer2DData.quadData.quadCount = 0;
    }

    Status createImage(
        VulkanDeviceData* deviceData, 
        uint32_t width, 
        uint32_t height, 
        VkFormat format, 
        VkImageTiling tiling,
        VkImageUsageFlags usageFlags, 
        VkMemoryPropertyFlags properties, 
        VkImage& image, 
        VkDeviceMemory& imageMemory) {

        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usageFlags;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

        if (vkCreateImage(deviceData->logicalDevice, &imageInfo, nullptr, &image) != VK_SUCCESS) {
            PONG_ERROR("failed to create image!");
            return Status::INITIALIZATION_FAILURE;
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(deviceData->logicalDevice, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        Buffers::findMemoryType(deviceData->physicalDevice, &allocInfo.memoryTypeIndex, memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(deviceData->logicalDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        vkBindImageMemory(deviceData->logicalDevice, image, imageMemory, 0);

        return Status::SUCCESS;
    }

    Status loadImage(Renderer* renderer, char const* imagePath, Texture2D& texture) {

        int width, height, channels = 0;

        stbi_uc* pixels = stbi_load(imagePath, &width, &height, &channels, STBI_rgb_alpha);

        VkDeviceSize imageSize = width * height * 4;

        if (!pixels) {
            PONG_ERROR("Failed to load in texture!");
            return Status::INITIALIZATION_FAILURE;
        }

        Buffers::BufferData bufferData;

        if (Buffers::createBuffer(
            renderer->deviceData.physicalDevice,
            renderer->deviceData.logicalDevice,
            imageSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            bufferData) != VK_SUCCESS) {

            PONG_ERROR("Failed to create buffer for texture");
            return Status::INITIALIZATION_FAILURE;
        }

        void* data; 
        vkMapMemory(renderer->deviceData.logicalDevice, bufferData.bufferMemory, 0, imageSize, 0, &data);
        memcpy(data, pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(renderer->deviceData.logicalDevice, bufferData.bufferMemory);

        stbi_image_free(pixels);

        createImage(
            &renderer->deviceData,
            width, 
            height, 
            VK_FORMAT_R8G8B8A8_SRGB, 
            VK_IMAGE_TILING_OPTIMAL, 
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
            texture.image,
            texture.memory
        );

        if (transitionImageLayout(renderer->deviceData.logicalDevice, renderer->deviceData.graphicsQueue,
            renderer->renderer2DData.commandPool, texture.image, VK_FORMAT_R8G8B8A8_SRGB,
            texture.layout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) != Status::SUCCESS) {
            return Status::INITIALIZATION_FAILURE;
        }
        texture.layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        copyBufferToImage(renderer->deviceData.logicalDevice, renderer->renderer2DData.commandPool,
            renderer->deviceData.graphicsQueue, bufferData.buffer, texture.image,
            static_cast<uint32_t>(width), static_cast<uint32_t>(height));
        if (transitionImageLayout(renderer->deviceData.logicalDevice, renderer->deviceData.graphicsQueue,
            renderer->renderer2DData.commandPool, texture.image, VK_FORMAT_R8G8B8A8_SRGB,
            texture.layout, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) != Status::SUCCESS) {
            return Status::INITIALIZATION_FAILURE;
        }

        vkDestroyBuffer(renderer->deviceData.logicalDevice, bufferData.buffer, nullptr);
        vkFreeMemory(renderer->deviceData.logicalDevice, bufferData.bufferMemory, nullptr);

        createImageView(renderer->deviceData.logicalDevice, texture.image, VK_FORMAT_R8G8B8A8_SRGB, texture.view);

        PONG_INFO("SUCCESSFULLY LOADED IMAGE!");

        return Status::SUCCESS;
    }
}