#include "vulkanUtils.h"
#include "logger.h"
#include "buffers.h"

namespace VulkanUtils {

    // Returns swapchain information supported by Vulkan.
    SwapchainSupportDetails querySwapchainSupport(
        VkPhysicalDevice device, 
        VkSurfaceKHR surface
    ) {

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
    
    // Returns information about the queue families available by our physical
    // device
    QueueFamilyIndices findQueueFamilies(
        VkPhysicalDevice device, 
        VkSurfaceKHR surface
    ) {
        // A struct for storing the index of the queue family that the 
        // device will be using
         QueueFamilyIndices indices;
    
         // Again, get the queue families that the device uses.
         uint32_t queueFamilyCount = 0;

         vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, 
                nullptr);
         
         VkQueueFamilyProperties* queueFamilies = new VkQueueFamilyProperties[queueFamilyCount];
    
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

        delete [] queueFamilies;
   
        return indices;
    }
    
    // Handles the creation and storage of swapchain data. 
    VkResult createSwapchain(
        SwapchainData* data,
        VulkanDeviceData* deviceData 
    ) {

        // Start by getting the supported formats for the swapchain
        SwapchainSupportDetails supportDetails = 
            querySwapchainSupport(deviceData->physicalDevice, 
            deviceData->surface);

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
        VkExtent2D chosenExtent = {
            static_cast<uint32_t>(deviceData->framebufferWidth), 
            static_cast<uint32_t>(deviceData->framebufferHeight)
        };

        // Make sure the width is not the maximum value of a 32-bit unsigned
        // integer. 
        if (supportDetails.capabilities.currentExtent.width != UINT32_MAX) {
            chosenExtent = supportDetails.capabilities.currentExtent;
        } else {

            // Make sure that the width and height of the images are greater than 0
            // and less than the maximum image dimensions.
            chosenExtent.width = std::clamp(chosenExtent.width,
                 supportDetails.capabilities.minImageExtent.width,
                 supportDetails.capabilities.maxImageExtent.width);

            chosenExtent.height = std::clamp(chosenExtent.height,
                 supportDetails.capabilities.minImageExtent.height,
                 supportDetails.capabilities.maxImageExtent.height);
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
        swapchainCreateInfo.surface = deviceData->surface;
        swapchainCreateInfo.minImageCount = imageCount;
        swapchainCreateInfo.imageFormat = chosenFormat.format;
        swapchainCreateInfo.imageColorSpace = chosenFormat.colorSpace;
        swapchainCreateInfo.imageExtent = chosenExtent;
        swapchainCreateInfo.imageArrayLayers = 1;
        swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        uint32_t queueFamilyIndices[] = {
            deviceData->indices.graphicsFamily.value(), 
            deviceData->indices.presentFamily.value()
        };
        
        if (deviceData->indices.graphicsFamily != deviceData->indices.presentFamily) {
            // If the present and graphics families are not the same then we 
            // specify that images can be owned by multiple queues at once.
            swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapchainCreateInfo.queueFamilyIndexCount = 2;           
            swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            // Specifies that an image can only be used by a single queue 
            // family at any given moment in time.
            swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            swapchainCreateInfo.queueFamilyIndexCount = 0;
            swapchainCreateInfo.pQueueFamilyIndices = nullptr;
        }
        
        // Can be used to specify a transform that all images in the swapchain 
        // will follow. In this case we'll just set it to the default.
        swapchainCreateInfo.preTransform = 
                supportDetails.capabilities.currentTransform;

        swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

        // Vulkan swapchains can become irrelevant when certain details are 
        // met (such as if the 
        // screen is resized). In this case we need to specify the old 
        // swapchain.
        swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
        
        swapchainCreateInfo.presentMode = chosenPresentMode;
        swapchainCreateInfo.clipped = VK_TRUE;

        // With all the config done, we can finally make the swapchain.
        VkSwapchainKHR swapchain;

        if (vkCreateSwapchainKHR(deviceData->logicalDevice, &swapchainCreateInfo, 
            nullptr, &swapchain) != VK_SUCCESS) {

            return VK_ERROR_INITIALIZATION_FAILED;
        }

        // Get the current images in the swapchain and store them for later 
        // use. 
        vkGetSwapchainImagesKHR(deviceData->logicalDevice, swapchain, 
            &imageCount, nullptr);

        VkImage* swapchainImages = new VkImage[imageCount];

        vkGetSwapchainImagesKHR(deviceData->logicalDevice, swapchain, &imageCount, 
            swapchainImages);

        // populate the swapchain data struct.
        data->swapchain = swapchain;
        data->imageCount = imageCount;
        data->swapchainFormat = chosenFormat.format;
        data->swapchainExtent = chosenExtent;
        data->pImages = swapchainImages;
        
        // Now we can create image views for use later on in the program.
        if (createImageViews(deviceData->logicalDevice, data) != VK_SUCCESS) {
            return VK_ERROR_INITIALIZATION_FAILED;
        }

        return VK_SUCCESS;
    }

    // A VkImageView object is required to use any Images in Vulkan.
    // A view describes how to access an image and which part of an image
    // should be accessed.
    VkResult createImageViews(VkDevice device, SwapchainData* data) {

        VkImageView* imageViews = new VkImageView[data->imageCount];

        for (size_t i = 0; i < data->imageCount; i++) {
            // We need to create a view for every image that we stored for
            // the swapChain.
            VkImageViewCreateInfo imageViewCreateInfo{};
            imageViewCreateInfo.sType 
                    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewCreateInfo.image = data->pImages[i];
            // Allows you to specify whether the image will be viewed as a 
            // 1D, 2D, or 3D texture.
            imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewCreateInfo.format = data->swapchainFormat;
            // Components field allow us to swizzle values around (force 
            // them to assume certain
            // values).
            // In this case we'll set the components to their default values.
            imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            // The subresourceRange field describes an image's purpose.
            // In our case our images will be used as color targets with no 
            // mipmapping levels
            // or layers.·
            imageViewCreateInfo.subresourceRange.aspectMask 
                    = VK_IMAGE_ASPECT_COLOR_BIT;
            imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
            imageViewCreateInfo.subresourceRange.levelCount = 1;
            imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
            imageViewCreateInfo.subresourceRange.layerCount = 1;
            // Create the image view and store it in the 
            // swapChainImageViews array.
            if (vkCreateImageView(device, &imageViewCreateInfo, nullptr, 
                &imageViews[i]) != VK_SUCCESS) {
                
                return VK_ERROR_INITIALIZATION_FAILED;
            }
        }

        data->pImageViews = imageViews;
        return VK_SUCCESS;
    }
    // Method to create renderpasses for our pipelines.
    VkResult createRenderPass(
        VkDevice device, 
        VkFormat format, 
        GraphicsPipelineData* data
    ) {
        // Struct or storing color and depth buffer information. 
        VkAttachmentDescription colorAttachment{};
        
        colorAttachment.format = format;
        // Used for multisampling
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        // Determines what happens to attachment contents before rendering. 
        // In our case we're going to clear the buffer before rendering.
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        // Determines what happens after rendering.
        // In our case we store contents in memory.
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        // Defines how image if formatted in memory. 
        // We leave it undefined in this case. 
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        // Defines the layout the image must be made into to be presentable for
        // the swapchain. 
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        // Next we define a subpass - a series of subcommands which control
        // how rendering occurs.

        // Define a reference to the color attachment above (for subpasses)
        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        
        // Now create the subpass:
        VkSubpassDescription subpass{};
        
        // Specify this subpass is used for graphics rendering
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        // Specify how many color attchments we have.
        subpass.colorAttachmentCount = 1;
        // Pass a pointer to the attachments reference.
        subpass.pColorAttachments = &colorAttachmentRef;

        // Now we define a dependency. This forces the subpass to wait until
        // a stage of the pipeline is done before executing again. 
        VkSubpassDependency dependency{};
        // Specifies the dependent subpass.
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        // The index of the subpass (we only have one)
        dependency.dstSubpass = 0;
        // Specifies which operations to wait on. In our case it's the color
        // attachment stage.
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        // Defines which stage to wait on. Ensures transitions only happen when 
        // absolutely necessary.
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        // Define render pass
        VkRenderPass renderPass;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        // Pass in the color attachment as an array
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        // Pass in the subpass
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        // Pass in the dependency
        renderPassInfo.pDependencies = &dependency;
        // Create the render pass
        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) 
                != VK_SUCCESS) {

            return VK_ERROR_INITIALIZATION_FAILED;
        }
        // Pass the render pass back to our storage struct.
        data->renderPass = renderPass;
        return VK_SUCCESS;
    }

    // Vulkan requires that you define your own graphics pipelines when you
    // want to use different combinations of shaders. This is because the·
    // graphics pipeline in Vulkan is almost completely immutable. This means
    // that the pipeline can be very well optimised (but will also require
    // a complete rewrite if you need anything different).

    VkResult createGraphicsPipeline(
        VkDevice device, 
        GraphicsPipelineData* data,
        const SwapchainData* swapchain
    ) {
        
        // Load out vertex and fragment shaders in machine readable bytecode.
        auto vert = readFile("src/shaders/vert.spv");
        auto frag = readFile("src/shaders/frag.spv");

        // Wrap the file contents in a shader module
        VkShaderModule vertShaderModule = createShaderModule(vert, device);
        VkShaderModule fragShaderModule = createShaderModule(frag, device);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType 
                = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        // This variable tells Vulkan to insert this into the vertex shader 
        // stage
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        // Specify the shader module to be used.
        vertShaderStageInfo.module = vertShaderModule;
        // Specify the entrypoint to the shader (i.e: the main function)
        vertShaderStageInfo.pName = "main";
        // Frag shader create info:
        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType 
                = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        // This variable tells Vulkan to insert this into the fragment 
        // shader stage
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        // Specify the shader module to be used.
        fragShaderStageInfo.module = fragShaderModule;
        // Specify the entrypoint to the shader (i.e: the main function)
        fragShaderStageInfo.pName = "main";
        // Store the stage information in an array for now - will be used 
        // later.
        VkPipelineShaderStageCreateInfo shaderStages[] = {
            vertShaderStageInfo,
            fragShaderStageInfo
        };

        // Now that we've loaded in the shaders we can start creating defining
        // how the pipeline will operate. 

        // Get the vertex buffer data for our triangle
        auto bindingDescription = Buffers::getBindingDescription();
        auto attributeDescriptions = Buffers::getAttributeDescriptions();

        // Defines how vertex data will be formatted in the shader.
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = 
                VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        // Describes details for loading vertex data.
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.vertexAttributeDescriptionCount = 
            static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        // Next, define the input assembly, or the kind of geometry drawn.
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = 
                VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        // Specifies that we'll draw a triangle from every 3 vertices
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        // Next we define the viewport - the region of the framebuffer that 
        // the output will be rendered to.
        VkViewport viewPort{};
        viewPort.x = 0.0f;
        viewPort.y = 0.0f;
        viewPort.width = (float) swapchain->swapchainExtent.width;
        viewPort.height = (float) swapchain->swapchainExtent.height;
        viewPort.minDepth = 0.0f;
        viewPort.maxDepth = 1.0f;

        // Next we define scissors. Scissors define the regions that the pixels
        // will be stored. These can be visualised as rectangles where anything
        // within the rectangle is drawn whilst everything else is discarded.
        VkRect2D scissor{};
        scissor.offset = {0,0};
        // We're ust making the scissor the size of the viewport
        scissor.extent = swapchain->swapchainExtent;

        // Now we combine the scissor and viewport into a single struct:
        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = 
                VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        // This takes in an array of viewports. Some GPUs allow multiple 
        // viewports to be specified, but this must be enabled as a device 
        // feature (in logical device section)
        viewportState.pViewports = &viewPort;
        viewportState.scissorCount = 1;
        // Same as with viewports - this takes in an array of scissors.·
        viewportState.pScissors = &scissor;

        // With the viewport defined, we can now define our rasteriser.
        // The rasterizer takes in the geometry shaped by the shader's vertices
        // and turns them into fragments. These fragments are then colored by
        // the fragment shader. The rasterizer also performs face culling and 
        // depth testing.
        
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = 
                VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        // This means that fragments below the near/far plane are clamped to 
        // the planes as opposed to discarding them. This requires a GPU 
        // feature in order for it to work.
        rasterizer.depthClampEnable = VK_FALSE;
        // Checks if geometry should pass through the rasterizer stage. We only
        // skip the stage if this is set to VK_TRUE.
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        // Determines how fragments are generated for geometry.
        // VK_POLYGON_MODE_FILL specifies that we want the area of the polygon 
        // to be filled with fragments
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        // Describes the thickness of lines. Any lines thicker than 1.0f 
        // requires a GPU feature to be enabled.
        rasterizer.lineWidth = 1.0f;
        // Determines the type of face-culling to use.
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        // Determines the vertex order for faces. Determines which are 
        // front-facing and which re back-facing. Can be clockwise or 
        // counter-clockwise.
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        // We also need to specify a MultiSampling struct. Multisampling allows
        // us to create effects like anti-aliasing in our programs.
        
        // Multisampling works by combining the fragment shader results into of
        // multiple polygons and rasterize them to the same pixel. This usually
        // occurs along edges where we get the most artifacting.
        
        // Enabling multisampling requires a GPU feauture to be enabled.
        // Multisampling will de disabled for now
        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = 
                VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        // Next stage is to define color blending. Color beldning defines
        // how what happens once the fragment shader returns a color. 
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        // Defines how th color will be formatted
        colorBlendAttachment.colorWriteMask = 
                VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT 
                | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE; 
        // Now we need to actually build the createInfo struct.·
        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = 
                VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        // Set this to VK_TRUE to enable bitwise blending.
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;

        // With all those defined, we now need to build a pipeline layout
        // struct. A pipeline layout struct details all the uniform values 
        // within our shaders.

        VkPipelineLayout pipelineLayout{};
        // Then instantiate the createInfo struct for that object:
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
        pipelineLayoutCreateInfo.sType = 
                VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.setLayoutCount = 0;
        pipelineLayoutCreateInfo.pSetLayouts = nullptr;
        pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
        pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

        // Now create the pipeline layout using the usual method:
        if (vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, 
                &pipelineLayout) != VK_SUCCESS) {
            return VK_ERROR_INITIALIZATION_FAILED;
        }

        // Now we can actually create the pipeline:
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        // Now we input all the structs that we defined previously into this 
        // one:
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = nullptr;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = nullptr;
        // Then we reference the layout struct.·
        pipelineInfo.layout = pipelineLayout;
        // Now we pass the renderPass into this.
        pipelineInfo.renderPass = data->renderPass;
        // We reference the subpass by index.·
        pipelineInfo.subpass = 0;
        
        // Finally, we can create our pipeline
        VkPipeline graphicsPipeline;

        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo,
                nullptr, &graphicsPipeline) != VK_SUCCESS) {
                return VK_ERROR_INITIALIZATION_FAILED;
        }

        // Delete the shader modules (doesn't need to happen during device 
        // cleanup phase)
        vkDestroyShaderModule(device, vertShaderModule, nullptr);
        vkDestroyShaderModule(device, fragShaderModule, nullptr);
        
        // Delete the pointers to the bytecode
        delete [] vert.p_byteCode;
        delete [] frag.p_byteCode;

        data->graphicsPipeline = graphicsPipeline;
        data->pipelineLayout = pipelineLayout;

        return VK_SUCCESS;
    }

    // Frame buffer creation method
    VkResult createFramebuffer(VkDevice device, VkFramebuffer* pFramebuffers,
            SwapchainData* swapchain, GraphicsPipelineData* graphicsPipeline) {
    
        for (size_t i = 0; i < swapchain->imageCount; i++) {

            // Get the image stored previously by our swapchain
            VkImageView attachments[] = {
                swapchain->pImageViews[i]
            };

            // Create a new framebuffer which uses our renderpass and image.
            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            // specify the renderpass - can only be done with a compatible 
            // render pass. This means that the number of attachments in the 
            // renderpass must be the same as the number of attachments in the 
            // framebuffer.
            framebufferInfo.renderPass = graphicsPipeline->renderPass;
            // Now we specify the image views associated with this framebuffer.
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = swapchain->swapchainExtent.width;
            framebufferInfo.height = swapchain->swapchainExtent.height;
            // Refers the the number of layers in image arrays.·
            framebufferInfo.layers = 1;

            // Create the framebuffer itself
            if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, 
                    &pFramebuffers[i]) != VK_SUCCESS) {
                return VK_ERROR_INITIALIZATION_FAILED;
            }

        }

        return VK_SUCCESS;
    }

    // Command buffer creation method
    VkResult createCommandBuffers(VkDevice device, VkCommandBuffer* buffers, 
        GraphicsPipelineData* pGraphicsPipeline, SwapchainData* pSwapchain, 
        VkFramebuffer* pFramebuffers, VkCommandPool commandPool, 
        Buffers::VertexBuffer* vertexBuffer, 
        Buffers::IndexBuffer* indexBuffer) {
        
        // We alocate command buffers by using a CommandBufferAllocationInfo struct.
        // // This struct specifies a command pool, as well as the number of buffers to
        // allocate.·
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        // Specifies that these buffers can be submitted to a queue for 
        // execution, but can't be called by other command buffers.
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = pSwapchain->imageCount;
  
        // Now we can start allocating our command buffers!
        if (vkAllocateCommandBuffers(device, &allocInfo, buffers) 
                != VK_SUCCESS) {
           
            return VK_ERROR_INITIALIZATION_FAILED;
        }

        // Now we need to start recording the command buffer. Recording a 
        // command buffer entails taking the draw commands and recording the 
        // same set of commands into them.

        for (size_t i = 0; i < pSwapchain->imageCount; i++) {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO; 
  
           if (vkBeginCommandBuffer(buffers[i], &beginInfo) != VK_SUCCESS) {
               return VK_ERROR_INITIALIZATION_FAILED;
           }
   
           // Now we can start setting up our render pass. Render passes are 
           // configured using a RenderPassBeginInfo struct:
  
           VkRenderPassBeginInfo renderPassInfo{};
           renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
           // Pass in our renderPass
           renderPassInfo.renderPass = pGraphicsPipeline->renderPass;
           // Get the specific renderpass.
           renderPassInfo.framebuffer = pFramebuffers[i];
           // These parameters define the size of the render area. Pixels 
           // outside the specified regions will have undefined values. For 
           // best performance, the render extent should match the size of the 
           // attachments.
           renderPassInfo.renderArea.offset = {0,0};
           renderPassInfo.renderArea.extent = pSwapchain->swapchainExtent;
           // Now we can define a clear color. This color is used as a load 
           // operation for the color attachment. In our case we're setting it 
           // to black.
           VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
           renderPassInfo.clearValueCount = 1;
           renderPassInfo.pClearValues = &clearColor;
 
           // Specify that render pass commands will be embedded in the primary 
           // command buffer. No secondary command buffers wll be executed.
           vkCmdBeginRenderPass(buffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
  
           // Once the render pass has started, we can now attach the graphics pipeline. The second·
           // parameter of this function call specifies whether this pipeline object is a graphics·
           // or compute pipeline.
           vkCmdBindPipeline(buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pGraphicsPipeline->graphicsPipeline);
            
           // Specify a list of vertex buffers that need to be recorded by the command buffer
           VkBuffer vertexBuffers[] = { vertexBuffer->bufferData.buffer };
           VkDeviceSize offsets[] = {0};

           // Bind the vertex buffers to the command buffer being recorded.
           vkCmdBindVertexBuffers(buffers[i], 0, 1, vertexBuffers, offsets);
           
           vkCmdBindIndexBuffer(buffers[i], indexBuffer->bufferData.buffer, 0, 
                VK_INDEX_TYPE_UINT16);
           
           // Now we can tell Vulkan to draw our triangle. This function has the parameters:
           vkCmdDrawIndexed(buffers[i], indexBuffer->indexCount, 1, 0, 0, 0);
  
           // Now we can end the render pass:
           vkCmdEndRenderPass(buffers[i]);
  
           //  Now we can end the command buffer recording
           if (vkEndCommandBuffer(buffers[i]) != VK_SUCCESS) {
               return VK_ERROR_INITIALIZATION_FAILED;
           }
        }

        return VK_SUCCESS;
    }

    VkResult recreateSwapchain(
        VulkanDeviceData* deviceData,
        SwapchainData* pSwapchain,
        GraphicsPipelineData* pGraphicsPipeline,
        VkCommandPool commandPool,
        VkFramebuffer* pFramebuffers,
        Buffers::VertexBuffer* vertexBuffer,
        Buffers::IndexBuffer* indexBuffer,
        VkCommandBuffer* pCommandbuffers
    ) {

        vkDeviceWaitIdle(deviceData->logicalDevice);

        cleanupSwapchain(deviceData->logicalDevice, pSwapchain, pGraphicsPipeline, 
            commandPool, pFramebuffers, pCommandbuffers);
        
        // Re-populate the swapchain
        if (createSwapchain(pSwapchain, deviceData) != VK_SUCCESS) {

            return VK_ERROR_INITIALIZATION_FAILED;
        }
        // Re-create our render pass
        if (createRenderPass(deviceData->logicalDevice, pSwapchain->swapchainFormat,
            pGraphicsPipeline) != VK_SUCCESS) {
            
            return VK_ERROR_INITIALIZATION_FAILED;
        }
        // Re-create our graphics pipeline
        if (createGraphicsPipeline(deviceData->logicalDevice, pGraphicsPipeline,
            pSwapchain) != VK_SUCCESS) {
         
            return VK_ERROR_INITIALIZATION_FAILED;
        }
        // Re-create our framebuffers
        if (createFramebuffer(deviceData->logicalDevice, pFramebuffers,
            pSwapchain, pGraphicsPipeline) != VK_SUCCESS) {

            return VK_ERROR_INITIALIZATION_FAILED;
        }
        // Re-create command buffers
        if (createCommandBuffers(deviceData->logicalDevice, pCommandbuffers,
            pGraphicsPipeline, pSwapchain, pFramebuffers, commandPool, 
            vertexBuffer, indexBuffer) 
            != VK_SUCCESS) {

            return VK_ERROR_INITIALIZATION_FAILED;
        }

        return VK_SUCCESS;
    }
    
    // Simple method for cleaning up all items relating to our swapchain
    void cleanupSwapchain(
        VkDevice device,
        SwapchainData* pSwapchain,
        GraphicsPipelineData* pGraphicsPipeline,
        VkCommandPool commandPool,
        VkFramebuffer* pFramebuffers,
        VkCommandBuffer* pCommandbuffers) {

        for (size_t i = 0; i < pSwapchain->imageCount; i++) {
            vkDestroyFramebuffer(device, pFramebuffers[i], nullptr);
        }

        vkFreeCommandBuffers(device, commandPool, pSwapchain->imageCount, 
                pCommandbuffers);

        // Destroy the graphics pipeline··
        vkDestroyPipeline(device, pGraphicsPipeline->graphicsPipeline, nullptr);
        // Clean up pipeline memory
        vkDestroyPipelineLayout(device, pGraphicsPipeline->pipelineLayout, nullptr);
        // Destroy the render pass
        vkDestroyRenderPass(device, pGraphicsPipeline->renderPass, nullptr);

        // Destroy image views
        for (size_t i = 0; i < pSwapchain->imageCount; i++) {
            vkDestroyImageView(device, pSwapchain->pImageViews[i], nullptr);
        }

        delete [] pSwapchain->pImageViews;

        delete [] pSwapchain->pImages;

        // Destroy the Swapchain
        vkDestroySwapchainKHR(device, pSwapchain->swapchain, nullptr);
    }

    // All shaders must be wrapped in a shader module. This is a helper 
    // function for wrapping the shader.·
    VkShaderModule createShaderModule(FileContents buffer, VkDevice &device) {
        // As is usually the case, we pass the config information to an info 
        // struct
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        // Add in the buffer size
        createInfo.codeSize = buffer.fileSize;
        // Add in the bytecode itself. The bytecode needs to be submitted in 
        // bytes, so
        // we need to cast this to a 32-bit unsigned integer to make this 
        // work.
        createInfo.pCode = 
                reinterpret_cast<const uint32_t*>(buffer.p_byteCode);
        // Create the shader
        VkShaderModule shader;
        if (vkCreateShaderModule(device, &createInfo, nullptr, &shader) 
                != VK_SUCCESS)        {
            // TODO: Find a better method to propagate errors.
            ERROR("Unsable to create shader module!\n");
        }
        // Return the shader
        return shader;
    }

    // Handles the creation of the triangle vertex buffer 
    VkResult createVertexBuffer(VulkanDeviceData* deviceData, 
        Buffers::VertexBuffer* vertexBuffer, VkCommandPool commandPool) {

        // Specify the required memory to store this buffer
        VkDeviceSize bufferSize = sizeof(vertexBuffer->vertices[0]) 
            * vertexBuffer->vertexCount;
        
        // Define a staging buffer. Since the most optimal memory for 
        // the GPU is not the same as that of the CPU, we define a buffer
        // that's most optimal for the GPU and then copy the data to the 
        // CPU's version. Thereby keeping them both on their most optimal
        // format.

        Buffers::BufferData stagingBuffer{};

        // Create a buffer for the staging buffer.
        if (Buffers::createBuffer(
            deviceData->physicalDevice, 
            deviceData->logicalDevice, 
            bufferSize, 
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
            stagingBuffer) != VK_SUCCESS) {

            return VK_ERROR_INITIALIZATION_FAILED;
        }
        
        // Now we need to fill the staging buffer:
        void* data;
        // Lets us access a region in memory by specifying an offset and size. 
        vkMapMemory(deviceData->logicalDevice, stagingBuffer.bufferMemory, 0, bufferSize, 
            0, &data);
        // Copy the memory from our vertex array into our mapped memory. 
        memcpy(data, vertexBuffer->vertices, (size_t)bufferSize);
        // Now unmap the memory
        vkUnmapMemory(deviceData->logicalDevice, stagingBuffer.bufferMemory);

        // Now create the actual buffer that we'll end up using:
        if (Buffers::createBuffer(
            deviceData->physicalDevice, 
            deviceData->logicalDevice, 
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
            vertexBuffer->bufferData) != VK_SUCCESS) {

            return VK_ERROR_INITIALIZATION_FAILED;
        }

        // Copy the contents of the staging buffer into the vertex buffer.
        Buffers::copyBuffer(
            deviceData->graphicsQueue, 
            deviceData->logicalDevice, 
            stagingBuffer.buffer, 
            vertexBuffer->bufferData.buffer, 
            bufferSize, commandPool
        );

        // Destroy the staging buffer and free memory
        vkDestroyBuffer(deviceData->logicalDevice, stagingBuffer.buffer, nullptr);
        vkFreeMemory(deviceData->logicalDevice, stagingBuffer.bufferMemory, nullptr);

        return VK_SUCCESS;
    }

    VkResult createIndexBuffer(VulkanDeviceData* deviceData, 
        Buffers::IndexBuffer* indexBuffer, VkCommandPool commandPool) {

        // We need to now get our memory for each index that our
        // index buffer will be drawing to
        VkDeviceSize bufferSize = sizeof(indexBuffer->indices[0])
                * indexBuffer->indexCount;
  
        // Initialise our staging buffer
        Buffers::BufferData stagingBuffer{};
        
        // Create the staging buffer
        if (Buffers::createBuffer(
            deviceData->physicalDevice, 
            deviceData->logicalDevice, 
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
            stagingBuffer) != VK_SUCCESS) {
   
            return VK_ERROR_INITIALIZATION_FAILED;
        }
  
        // Now we need to fill the staging buffer:
        void* data;
        // Lets us access a region in memory by specifying an offset and size.
        vkMapMemory(deviceData->logicalDevice, stagingBuffer.bufferMemory, 0, bufferSize,
            0, &data);
        // Copy the memory from our vertex array into our mapped memory.
        memcpy(data, indexBuffer->indices, (size_t)bufferSize);
        // Now unmap the memory
        vkUnmapMemory(deviceData->logicalDevice, stagingBuffer.bufferMemory);
        
        // Create the actual buffer that we'll end up using
        if (Buffers::createBuffer(
            deviceData->physicalDevice, 
            deviceData->logicalDevice, 
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
            indexBuffer->bufferData) != VK_SUCCESS) {
   
            return VK_ERROR_INITIALIZATION_FAILED;
        }
   
        // Copy the contents of the staging buffer into the index buffer
        Buffers::copyBuffer(
            deviceData->graphicsQueue, 
            deviceData->logicalDevice,
            stagingBuffer.buffer, 
            indexBuffer->bufferData.buffer, 
            bufferSize, commandPool);

        // Destroy the staging buffer and de-allocate its memory
        vkDestroyBuffer(deviceData->logicalDevice, stagingBuffer.buffer, nullptr);
        vkFreeMemory(deviceData->logicalDevice, stagingBuffer.bufferMemory, nullptr);
   
        return VK_SUCCESS;

    }
}
