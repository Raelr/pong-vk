/*
 *  This file contains all data that's closely related to general Vulkan struct creation.
 *  The point is to act as a utility file for quick initialisers for the renderer.
 * */

#ifndef PONG_VK_INITIALISERS_H
#define PONG_VK_INITIALISERS_H

#include <vulkan/vulkan.h>
#include "vulkanDeviceData.h"
#include "swapchainData.h"

namespace Renderer {

    // TODO: Change return types to Status
    VkApplicationInfo initialiseVulkanApplicationInfo(const char*, const char*, uint32_t, uint32_t, uint32_t);
    VkResult createSwapchain(SwapchainData*, VulkanDeviceData*);
    VkResult createImageViews(VkDevice, SwapchainData*);
    Status createImageView(VkDevice, VkImage, VkFormat, VkImageView&);
    Status initialiseVulkanInstance(VulkanDeviceData*, bool, const char*, const char*);
    Status createVulkanDeviceData(VulkanDeviceData*, GLFWwindow*, bool);
    VkDescriptorPoolSize initialisePoolSize(VkDescriptorType, uint32_t);
    VkDescriptorSetLayoutBinding initiialiseDescriptorSetLayoutBinding(uint32_t, VkDescriptorType, uint32_t,
        VkShaderStageFlags);
    VkWriteDescriptorSet initialiseWriteDescriptorSet(VkDescriptorSet, VkDescriptorType, uint32_t,
        uint32_t, VkDescriptorBufferInfo* = nullptr, VkDescriptorImageInfo* = nullptr);
    VkPipelineShaderStageCreateInfo initialisePipelineShaderStageCreateInfo(
        VkShaderStageFlagBits, VkShaderModule, const char*);
    VkSampler initialiseSampler(
        VkDevice, VkFilter, VkFilter, VkSamplerAddressMode,
        VkBorderColor, VkCompareOp, VkSamplerMipmapMode,
        VkBool32 = VK_FALSE, VkBool32 = VK_TRUE,
        VkBool32 = VK_FALSE
    );
    VkDescriptorBufferInfo initialiseDescriptorBufferInfo(VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range);
    VkDescriptorImageInfo initialiseDescriptorImageInfo(VkImageLayout imageLayout, VkImageView imageView, VkSampler imageSampler);
}

#endif //PONG_VK_INITIALISERS_H
