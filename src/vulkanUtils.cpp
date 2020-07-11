#include "vulkanUtils.h"
#include "logger.h"
//struct SwapchainData {
//    VkSwapchainKHR swapchain;
//    uint32_t imageCount;
//    VkImage* swapchainImages;
//    VkFormat swapchainFormat;
//    VkExtent2D swapchainExtent;
//};

//struct SwapChainSupportDetails2 {
//    VkSurfaceCapabilitiesKHR capabilities;
//    std::vector<VkSurfaceFormatKHR> formats;
//    std::vector<VkPresentModeKHR> presentModes;
//};

namespace VulkanUtils {

    SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device, 
            VkSurfaceKHR surface){

        // instantiate a struct to store swapchain details.
        SwapchainSupportDetails details;

        // Now follow a familiar pattern and query all the support details 
        // from Vulkan...
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, 
                &details.capabilities);

        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, 
                details.formats.data());

        // Using a vector for the utility functions - statically resize the 
        // data within it to hold·the data we need.
        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount,
                    details.formats.data());
        }
       
        uint32_t presentModeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, 
                &presentModeCount, nullptr);

        // Same as above ^
        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, 
                    &presentModeCount,details.presentModes.data());
        }
       
        // Return the details we need
        return details;

    }

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, 
            VkSurfaceKHR surface) {

        // A struct for storing the index of the queue family that the device        will be using
         QueueFamilyIndices indices;
    
         // Again, get the queue families that the device uses.
         uint32_t queueFamilyCount = 0;

         vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, 
                nullptr);
         
         VkQueueFamilyProperties queueFamilies[queueFamilyCount];
    
         vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, 
                queueFamilies);
    
         // Now that we know the families, we can now assess the suitability 
         // of this device.
         for (size_t i = 0; i < queueFamilyCount; i++) {
             // We search for a flag which specifies that the queue supports 
             // graphics operations.
             // This is specified with the VK_QUEUE_GRAPHICS_BIT flag.
             if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                 indices.graphicsFamily = i;
             }
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, 
                    &presentSupport);
    
             if (presentSupport) {
                 indices.presentFamily = i;
            }
        }
   
        return indices;
    }

    SwapchainData createSwapchain(VkPhysicalDevice device, 
            VkSurfaceKHR surface, const uint32_t windowHeight, 
            const uint32_t windowWidth,
            QueueFamilyIndices indices) {
        
        SwapchainData data{};

        // Start by getting the supported formats for the swapchain
        SwapchainSupportDetails supportDetails = 
                querySwapchainSupport(device, surface);

        // We want to find three settings for our swapchain:
        // 1. We want to find the surface format (color depth).
        // 2. Presentation Mode (conditions for 'swapping' images to the 
        // screen - kinda like vSync).
        // 3. Swap Extent (resolution of images in the swapchain)

        // First lets get the format, we'll set it to the first format 
        // in the list to start:
        VkSurfaceFormatKHR chosenFormat = supportDetails.formats[0];

        // The Format struct contains two variables that should be set:
        // 1. Format - The color channels and types used by the Vulkan.
        // 2. Colorspace - Checks if the SRGB color space is supported or not

        for (auto& format : supportDetails.formats) {
        
            // We'll be looking for the SRGB color space since it results in 
            // more accurate perceived colors.
            // Each color channel will be stored in 8 bit integers (32 bits
            // total)
            if (format.format == VK_FORMAT_B8G8R8A8_SRGB && 
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {

                chosenFormat = format;
                INFO("Found SRGB channel for rendering format");
                break;
                // TODO: Might be good to have a fallback for when we fail to
                // find the desired color space. 
            }
        }
        
        // We set the mode to vsync as a start.
        VkPresentModeKHR chosenPresentMode = VK_PRESENT_MODE_FIFO_KHR;


        // Ideally, we want to use triple buffering as it results in less screen
        // tear and less performance issues than normal vsync. The present
        // mode we're looking for here is the VL_PRESENT_MODE_MAILBOX_KHR.
        for (auto& presentMode : supportDetails.presentModes) {
            // If we can get triple buffering instead of vsync then we'll take
            // it. 
            if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {

                chosenPresentMode = presentMode;
                INFO("Triple buffering enabled for present mode!");
                break;
            }
        }

        // Set the swap extent, or the resolution of the images being processed
        // by the swapchain.
        VkExtent2D chosenExtent = {windowWidth, windowHeight};

        // Make sure that the width and height of the images are greater than 0
        // and less than the maximum image dimensions.
        chosenExtent.width = std::clamp(chosenExtent.width, 
            supportDetails.capabilities.minImageExtent.width,
            supportDetails.capabilities.maxImageExtent.width);

        chosenExtent.height = std::clamp(chosenExtent.height,
            supportDetails.capabilities.minImageExtent.height,
            supportDetails.capabilities.maxImageExtent.height);

        // Make sure the width is not the maximum value of a 32-bit unsigned
        // integer. 
        if (supportDetails.capabilities.currentExtent.width != UINT32_MAX) {
            chosenExtent = supportDetails.capabilities.currentExtent;
        }

        INFO("Device extent has been set to: [ " + 
            std::to_string(chosenExtent.width) + ", " + 
            std::to_string(chosenExtent.height) + " ]");

        // Now we handle the actual creation of the swapchain:

        // First, we need to specify how many images the swapchain will handle:
        uint32_t imageCount = supportDetails.capabilities.minImageCount + 1; 
        // We add an additional image just to allow for some extra flexibility.
        
        // Check that we're assigning the correct number of images for the 
        // queue. A maxImageCount of 0 implies that there is no max.
        if (supportDetails.capabilities.maxImageCount > 0
        && imageCount > supportDetails.capabilities.maxImageCount) {
            // Set the image count to the maximum allowed in the queue.
            imageCount = supportDetails.capabilities.maxImageCount;
        }
        
        // Now that we've set the images, we can start setting up our swapchain
        // configuration.
        VkSwapchainCreateInfoKHR swapchainCreateInfo {};
        swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchainCreateInfo.surface = surface;
        swapchainCreateInfo.minImageCount = imageCount;
        swapchainCreateInfo.imageFormat = chosenFormat.format;
        swapchainCreateInfo.imageColorSpace = chosenFormat.colorSpace;
        swapchainCreateInfo.imageExtent = chosenExtent;
        swapchainCreateInfo.imageArrayLayers = 1;
        swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        return data;

    }

}
