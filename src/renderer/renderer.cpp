#include "renderer.h"
#include <cstring>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include "vk/initialisers.h"

namespace Renderer {

    static const char* validationLayers[] = {
        "VK_LAYER_KHRONOS_validation"
    };

    static const char* deviceExtensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    Status initialiseRenderer(Renderer* renderer, bool enableValidationLayers, GLFWwindow* window) {

        // ========================== VALIDATION LAYER CHECKING ==============================

        uint32_t layerCount = 0;

        // Get the number of layers available for vulkan.
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        VkLayerProperties layerProperties[layerCount];

        if (enableValidationLayers) {

            if (checkValidationLayerSupport(layerCount,layerProperties, renderer->deviceData.validationLayers,
                renderer->deviceData.validationLayerCount) != Status::SUCCESS) {
                return Status::FAILURE;
            }
        }

        // ======================= VULKAN INSTANCE CREATION ==================================

        if (checkVulkanExtensions(&renderer->deviceData, enableValidationLayers) == Status::FAILURE) {
            ERROR("No GLFW extensions available!");
            return Status::FAILURE;
        }

        if (initialiseVulkanInstance(&renderer->deviceData, enableValidationLayers, "Pong", "no engine") == Status::FAILURE) {
            return Status::INITIALIZATION_FAILURE;
        }

        INFO("Initialised Vulkan instance.");

        if (enableValidationLayers) {
            if (initialiseDebugUtilsMessenger(renderer->deviceData.instance, &renderer->deviceData.debugMessenger)
                != Status::SUCCESS) {
                ERROR("Failed to create Debug utils messenger!");
                return Status::INITIALIZATION_FAILURE;
            }
        }

        INFO("Created Debug Utils Messenger");

        // ============================ SURFACE CREATION ====================================

        if (createWindowSurface(renderer->deviceData.instance, window, &renderer->deviceData.surface)
            != Status::SUCCESS) {
            return Status::INITIALIZATION_FAILURE;
        }

        INFO("Retrieved Surface from GLFW.");

        // ========================= PHYSICAL DEVICE CREATION ===============================

        if (createPhysicalDevice(renderer) != Status::SUCCESS) {
            ERROR("Failed to create physical device!");
            return Status::INITIALIZATION_FAILURE;
        }

        INFO("Created physical device!");

        // ========================== LOGICAL DEVICE CREATION ===============================

        if (createLogicalDevice(renderer) != Status::SUCCESS) {
            return Status::INITIALIZATION_FAILURE;
        }

        INFO("Created logical device!");

        // ============================= SWAPCHAIN CREATION =================================

        glfwGetFramebufferSize(window, &renderer->deviceData.framebufferWidth,
                               &renderer->deviceData.framebufferHeight);

        // Create the swapchain (should initialise both the swapchain and image views)
        if (createSwapchain(&renderer->swapchainData, &renderer->deviceData) != VK_SUCCESS) {
            ERROR("Failed to create swapchain!");
            return Status::FAILURE;
        }

        INFO("Initialised Swapchain");

        // ================================= RENDERER 2D ====================================

        if (!Renderer2D::initialiseRenderer2D(&renderer->deviceData,
            &renderer->renderer2DData, renderer->swapchainData)) {
            ERROR("Failed to create renderer2D");
            return Status::INITIALIZATION_FAILURE;
        }

        INFO("Initialised renderer2D!");

        // =============================== COMMAND BUFFERS ==================================

        renderer->commandBuffers = static_cast<VkCommandBuffer *>(malloc(
                renderer->swapchainData.imageCount * sizeof(VkCommandBuffer)));

        // With the command pool created, we can now start creating and allocating
        // command buffers. Because these commands involve allocating a framebuffer,
        // we'll need to record a command buffer for every image in the swap chain.

        // It should be noted that command buffers are automatically cleaned up when
        // the commandpool is destroyed. As such they require no explicit cleanup.

        if (createCommandBuffers(
                renderer->deviceData.logicalDevice,
                renderer->commandBuffers,
                &renderer->renderer2DData.graphicsPipeline,
                &renderer->swapchainData,
                renderer->renderer2DData.frameBuffers,
                renderer->renderer2DData.commandPool,
                &renderer->renderer2DData.quadData.vertexBuffer,
                &renderer->renderer2DData.quadData.indexBuffer,
                renderer->renderer2DData.quadData.descriptorSets,
                renderer->renderer2DData.quadData.quadCount)
            != VK_SUCCESS) {

            ERROR("Failed to create command buffers!");
            return Status::INITIALIZATION_FAILURE;
        }

        // ================================ SYNC OBJECTS ====================================

        createSyncObjects(renderer);

        INFO("Created synchronisation objects");

        return Status::SUCCESS;
    }

    Status createWindowSurface(VkInstance instance, GLFWwindow* window, VkSurfaceKHR* surface) {
        if (glfwCreateWindowSurface(instance, window, nullptr,
                                    surface) != VK_SUCCESS) {
            ERROR("Failed to create window surface!");
            return Status::INITIALIZATION_FAILURE;
        }

        return Status::SUCCESS;
    }

    Status createPhysicalDevice(Renderer* renderer) {

        renderer->deviceData.physicalDevice = VK_NULL_HANDLE;

        // Begin by querying which devices are supported by Vulkan on this machine.
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(renderer->deviceData.instance, &deviceCount, nullptr);

        if (deviceCount == 0) {
            ERROR("Failed to find GPUs that support Vulkan!");
            return Status::FAILURE;
        }

        // Now we query which specific devices we have
        VkPhysicalDevice devices[deviceCount];
        vkEnumeratePhysicalDevices(renderer->deviceData.instance, &deviceCount, devices);

        Status status = Status::FAILURE;

        // Now search for the GPU we want.
        for (size_t i = 0; i < deviceCount; i++) {

            // We need a device that has the queue families that we need.
            QueueFamilyIndices indices = findQueueFamilies(devices[i],
                renderer->deviceData.surface);

            // Now we query all available extensions on this device
            uint32_t extensionCount = 0;
            vkEnumerateDeviceExtensionProperties(devices[i], nullptr, &extensionCount, nullptr);

            VkExtensionProperties availableExtensions[extensionCount];
            vkEnumerateDeviceExtensionProperties(devices[i], nullptr, &extensionCount, availableExtensions);

            // Check if our device has all our required extensions
            size_t found = 0;
            for (size_t j = 0; j < renderer->deviceData.deviceExtensionCount; j++) {
                for (size_t k = 0; k < extensionCount; k++) {
                    if (strcmp(renderer->deviceData.deviceExtensions[j], availableExtensions[k].extensionName) == 0) {
                        found++;
                    }
                }
            }

            bool extensionsSupported = (found == renderer->deviceData.deviceExtensionCount);
            bool swapchainAdequate = false;

            if (extensionsSupported) {
                // Get the swap-chain details
                SwapchainSupportDetails supportDetails = querySwapchainSupport(devices[i], renderer->deviceData.surface);
                // Make sure that we have at least one supported format and one supported presentation mode.
                swapchainAdequate = supportDetails.formatCount > 0 && supportDetails.presentModesCount > 0;

                free(supportDetails.formats);
                free(supportDetails.presentModes);
            }

            bool is_supported = (
                indices.graphicsFamily.has_value()
                && indices.presentFamily.has_value())
                // If all available extensions were 'ticked off' the set then we know we have all
                // required extensions.
                && extensionsSupported
                && swapchainAdequate;
            // If the device has all our required extensions and has a valid swap-chain and has our
            // required queue families, then we can use that device for rendering!
            if (is_supported) {
                renderer->deviceData.physicalDevice = devices[i];
                status = Status::SUCCESS;
                break;
            }
        }

        return status;
    }

    Status createLogicalDevice(Renderer* pRenderer) {

        // First, we need to query which queue families our physical device supports.
        pRenderer->deviceData.indices = findQueueFamilies(pRenderer->deviceData.physicalDevice,
            pRenderer->deviceData.surface);

        // It's possible that multiple queues are actually the same (such as graphics and present
        // queues). We therefore check if the two that we're looking for are the same.
        size_t createSize = (pRenderer->deviceData.indices.graphicsFamily.value()
                             == pRenderer->deviceData.indices.presentFamily.value()) ? 1 : 2;

        // Now we need an array for storing the queues that we want to create in future.
        VkDeviceQueueCreateInfo createInfos[createSize];
        uint32_t uniqueFamilies[createSize];

        uniqueFamilies[0] = pRenderer->deviceData.indices.graphicsFamily.value();

        // If we have multiple graphics families then add those into the array
        if (createSize == 2) {
            uniqueFamilies[1] = pRenderer->deviceData.indices.presentFamily.value();
        }

        // Each queue is given a priority - we'll set these ones to their maximum value
        float priority = 1.0f;

        // Now create a queue creation struct for each unique family that we have
        for (size_t i = 0; i < createSize; i++) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = uniqueFamilies[i];
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &priority;
            createInfos[i] = queueCreateInfo;
        }

        // Leave this empty for now - can add things later when we need.
        VkPhysicalDeviceFeatures deviceFeatures{};

        // Now we need to actually configure the logical device (note that it uses the queue info
        // and the device features we defined earlier).
        VkDeviceCreateInfo logicalDeviceInfo{};
        logicalDeviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        logicalDeviceInfo.pQueueCreateInfos = createInfos;
        logicalDeviceInfo.queueCreateInfoCount = static_cast<uint32_t>(createSize);
        logicalDeviceInfo.pEnabledFeatures = &deviceFeatures;
        logicalDeviceInfo.enabledExtensionCount = pRenderer->deviceData.deviceExtensionCount;
        logicalDeviceInfo.ppEnabledExtensionNames = pRenderer->deviceData.deviceExtensions;

        // Now we create the logical device using the data we've accumulated thus far.
        if (vkCreateDevice(pRenderer->deviceData.physicalDevice, &logicalDeviceInfo,nullptr,
                &pRenderer->deviceData.logicalDevice) != VK_SUCCESS) {
            ERROR("Failed to create logical device!");
            return Status::INITIALIZATION_FAILURE;
        }

        // Now we just need to create the queue which will be used for our commands.

        // Create the queue using the struct and logical device we created earlier.
        vkGetDeviceQueue(pRenderer->deviceData.logicalDevice,
             pRenderer->deviceData.indices.graphicsFamily.value(), 0,
             &pRenderer->deviceData.graphicsQueue);
        // Create the presentation queue using the struct.
        vkGetDeviceQueue(pRenderer->deviceData.logicalDevice,
             pRenderer->deviceData.indices.presentFamily.value(), 0,
             &pRenderer->deviceData.presentQueue);

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
            ERROR("Unable to load in validation layers! Invalid validation layers have been provided");
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
            pRenderer->commandBuffers,
            pRenderer->renderer2DData.quadData.uniformBuffers,
            pRenderer->renderer2DData.descriptorPool,
            pRenderer->renderer2DData.quadData.quadCount
        );

        Renderer2D::cleanupRenderer2D(&pRenderer->deviceData, &pRenderer->renderer2DData);

        free(pRenderer->deviceData.extensions);
        free(pRenderer->renderer2DData.frameBuffers);

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
        free(pRenderer->commandBuffers);

        // Free memory associated with descriptor sets and uniform buffers
        for (size_t i = 0; i < pRenderer->renderer2DData.quadData.quadCount; i++) {
            free(pRenderer->renderer2DData.quadData.descriptorSets[i]);
            free(pRenderer->renderer2DData.quadData.uniformBuffers[i]);
        }

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

                ERROR("Failed to create synchronisation objects for frame!");
                return Status::INITIALIZATION_FAILURE;
            }
        }

        return Status::SUCCESS;
    }

    Status drawFrame(Renderer* pRenderer, bool* resized, GLFWwindow* window) {

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

            vkResetCommandBuffer(pRenderer->commandBuffers[pRenderer->imageIndex], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

            // TODO: Change this to just accept a renderer.
            if (createCommandBuffer(
                    pRenderer->deviceData.logicalDevice,
                    &pRenderer->commandBuffers[pRenderer->imageIndex],
                    pRenderer->imageIndex,
                    &pRenderer->renderer2DData.graphicsPipeline,
                    &pRenderer->swapchainData,
                    pRenderer->renderer2DData.frameBuffers,
                    &pRenderer->renderer2DData.commandPool,
                    &pRenderer->renderer2DData.quadData.vertexBuffer,
                    &pRenderer->renderer2DData.quadData.indexBuffer,
                    pRenderer->renderer2DData.quadData.descriptorSets,
                    pRenderer->renderer2DData.quadData.quadCount) != VK_SUCCESS) {

                ERROR("Failed to create command buffers!");
                return Status::FAILURE;
            }
        }

        // If our swapchain is out of date (no longer valid, then we re-create
        // it)
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {

            onWindowMinimised(window, &pRenderer->deviceData.framebufferWidth,
                               &pRenderer->deviceData.framebufferHeight);

            // Re-create the swap chain in its entirety if the pipeline is no
            // longer valid or is out of date.
            recreateSwapchain(
                &pRenderer->deviceData,
                &pRenderer->swapchainData,
                &pRenderer->renderer2DData.graphicsPipeline,
                pRenderer->renderer2DData.commandPool,
                pRenderer->renderer2DData.frameBuffers,
                &pRenderer->renderer2DData.quadData.vertexBuffer,
                &pRenderer->renderer2DData.quadData.indexBuffer,
                pRenderer->commandBuffers,
                &pRenderer->renderer2DData.quadData.descriptorSetLayout,
                &pRenderer->renderer2DData.descriptorPool,
                pRenderer->renderer2DData.quadData.descriptorSets,
                pRenderer->renderer2DData.quadData.uniformBuffers,
                pRenderer->renderer2DData.quadData.quadCount);

            *resized = false;
            // Go to the next iteration of the loop
            return Status::SKIPPED_FRAME;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            ERROR("Failed to acquire swapchain image!");
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
        submitInfo.pCommandBuffers = &pRenderer->commandBuffers[pRenderer->imageIndex];
        // Now we specify which semaphores we need to signal once our command buffers
        // have finished execution.
        VkSemaphore signalSemaphores[] = {pRenderer->renderFinishedSemaphores[pRenderer->currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        // We need to reset the fence to an unsignalled state before moving on.
        vkResetFences(pRenderer->deviceData.logicalDevice, 1, &pRenderer->inFlightFences[pRenderer->currentFrame]);

        // Finally, we submit the buffer to the graphics queue
        if (vkQueueSubmit(pRenderer->deviceData.graphicsQueue, 1, &submitInfo, pRenderer->inFlightFences[pRenderer->currentFrame])
            != VK_SUCCESS) {
            ERROR("Failed to submit draw command buffer!");
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

            onWindowMinimised(window, &pRenderer->deviceData.framebufferWidth,
                               &pRenderer->deviceData.framebufferHeight);

            // Re-create the swap chain in its entirety if the pipeline is no
            // longer valid or is out of date.
            recreateSwapchain(
                &pRenderer->deviceData,
                &pRenderer->swapchainData,
                &pRenderer->renderer2DData.graphicsPipeline,
                pRenderer->renderer2DData.commandPool,
                pRenderer->renderer2DData.frameBuffers,
                &pRenderer->renderer2DData.quadData.vertexBuffer,
                &pRenderer->renderer2DData.quadData.indexBuffer,
                pRenderer->commandBuffers,
                &pRenderer->renderer2DData.quadData.descriptorSetLayout,
                &pRenderer->renderer2DData.descriptorPool,
                pRenderer->renderer2DData.quadData.descriptorSets,
                pRenderer->renderer2DData.quadData.uniformBuffers,
                pRenderer->renderer2DData.quadData.quadCount);

            *resized = false;
            return Status::SKIPPED_FRAME;
        } else if (result != VK_SUCCESS) {
            ERROR("Failed to present swapchain image!");
            return Status::FAILURE;
        }

        // This should clamp the value of currentFrame between 0 and 1.
        pRenderer->currentFrame = (pRenderer->currentFrame + 1) % pRenderer->maxFramesInFlight;

        // WORK OUT WHAT TO DO WITH UNIFORM BUFFERS

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

    // A wrapper method for quad drawing.
    Status registerQuad2D(Renderer* pRenderer) {

        Status success = (Renderer2D::queueQuad(
                &pRenderer->renderer2DData,
                &pRenderer->deviceData,
                &pRenderer->swapchainData)) ? Status::SUCCESS : Status::INITIALIZATION_FAILURE;

        return success;
    }

    Status drawQuad(Renderer* pRenderer, glm::vec3 pos, glm::vec3 rot, float degrees, glm::vec3 scale, uint32_t objectIndex) {

        Buffers::UniformBufferObject ubo{};

        ubo.mvp = glm::translate(ubo.mvp, pos);
        ubo.mvp = glm::rotate(ubo.mvp, degrees, rot);
        ubo.mvp = glm::scale(ubo.mvp, scale);

        // Set the view
        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -1.0f));

        glm::mat4 proj = glm::ortho(
                -(static_cast<float>(pRenderer->swapchainData.swapchainExtent.width) / 2),
                static_cast<float>(pRenderer->swapchainData.swapchainExtent.width) / 2,
                static_cast<float>(pRenderer->swapchainData.swapchainExtent.height) / 2,
                -(static_cast<float>(pRenderer->swapchainData.swapchainExtent.height) / 2), -1.0f, 1.0f);

        // Rotate the model matrix
        ubo.mvp = proj * view * ubo.mvp;

        uint32_t imageIdx = pRenderer->imageIndex;
        VkDeviceMemory memory = pRenderer->renderer2DData.quadData.uniformBuffers[objectIndex][imageIdx].bufferMemory;

        // Now we bind our data to the UBO for later use
        void* data;
        vkMapMemory(pRenderer->deviceData.logicalDevice, memory,0, sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(pRenderer->deviceData.logicalDevice, memory);

        return Status::SUCCESS;
    }
}