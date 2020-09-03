#include "vulkanUtils.h"
#include "buffers.h"
#include "initialisers.h"

namespace Renderer {

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

    VkResult createDescriptorSetLayout( VkDevice device,
        VkDescriptorSetLayout* descriptorSetLayout) {
        
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        // Aligns to the binding specified in our shader.
        uboLayoutBinding.binding = 0;
        // Specifies the type of descriptor
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        // How many descriptors there are (if appplicable)
        uboLayoutBinding.descriptorCount = 1;
        // Which stage this will be used in (the vertex stage)
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        uboLayoutBinding.pImmutableSamplers = nullptr;

        // Now specify the struct for configuring the descriptor set.
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &uboLayoutBinding;

        // Create the descriptor set
        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, 
            descriptorSetLayout) != VK_SUCCESS) {
            
            return VK_ERROR_INITIALIZATION_FAILED;
        }

        return VK_SUCCESS;
    }
    
    VkResult createDescriptorPool(
        VkDevice device,
        uint32_t imageCount,
        VkDescriptorPool* descriptorPool,
        size_t objectCount
    ) {

        VkDescriptorPoolSize sizes[objectCount];
        
        for (int i = 0; i < objectCount; i++) {
            
            VkDescriptorPoolSize poolSize{};
            poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            poolSize.descriptorCount = imageCount * objectCount;

            sizes[i] = poolSize;
        }

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = objectCount;
        poolInfo.pPoolSizes = sizes;
        poolInfo.maxSets = imageCount * objectCount;
        
        if (vkCreateDescriptorPool(device, &poolInfo, nullptr, descriptorPool)
            != VK_SUCCESS) {
            return VK_ERROR_INITIALIZATION_FAILED;
        }

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
        const SwapchainData* swapchain,
        VkDescriptorSetLayout* descriptorSetLayout
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
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
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

        VkPushConstantRange range = {};
        range.stageFlags = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
        range.offset = 0;
        range.size = sizeof(Buffers::UniformBufferObject);

        // With all those defined, we now need to build a pipeline layout
        // struct. A pipeline layout struct details all the uniform values 
        // within our shaders.

        VkPipelineLayout pipelineLayout{};
        // Then instantiate the createInfo struct for that object:
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
        pipelineLayoutCreateInfo.sType = 
                VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.setLayoutCount = 1;
        pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayout;
        pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
        pipelineLayoutCreateInfo.pPushConstantRanges = &range;

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
        Buffers::VertexBuffer* vertexBuffer, Buffers::IndexBuffer* indexBuffer, 
        VkDescriptorSet** descriptorSets, size_t objectCount) {
        
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
            
           for (size_t j = 0; j < objectCount; j++) {

                vkCmdBindDescriptorSets(buffers[i],
                VK_PIPELINE_BIND_POINT_GRAPHICS, 
                pGraphicsPipeline->pipelineLayout, 0, 1, &descriptorSets[j][i],
                0, nullptr);

                // Now we can tell Vulkan to draw our triangle. This function has the parameters:
                vkCmdDrawIndexed(buffers[i], indexBuffer->indexCount, 1, 0, 0, 0);
           } 
           // Now we can end the render pass:
           vkCmdEndRenderPass(buffers[i]);
  
           //  Now we can end the command buffer recording
           if (vkEndCommandBuffer(buffers[i]) != VK_SUCCESS) {
               return VK_ERROR_INITIALIZATION_FAILED;
           }
        }

        return VK_SUCCESS;
    }

    // Command buffer creation method
    VkResult rerecordCommandBuffer(
            VkDevice device, VkCommandBuffer* buffer, size_t bufferIndex,
            GraphicsPipelineData* pGraphicsPipeline, SwapchainData* pSwapchain,
            VkFramebuffer* pFramebuffers, VkCommandPool* commandPool,
            Buffers::VertexBuffer* vertexBuffer, Buffers::IndexBuffer* indexBuffer,
            VkDescriptorSet** descriptorSets, size_t objectCount) {

        // We allocate command buffers by using a CommandBufferAllocationInfo struct.
        // // This struct specifies a command pool, as well as the number of buffers to
        // allocate.·
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = *commandPool;
        // Specifies that these buffers can be submitted to a queue for
        // execution, but can't be called by other command buffers.
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = pSwapchain->imageCount;

        // Now we need to start recording the command buffer. Recording a
        // command buffer entails taking the draw commands and recording the
        // same set of commands into them.

            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            if (vkBeginCommandBuffer(*buffer, &beginInfo) != VK_SUCCESS) {
                return VK_ERROR_INITIALIZATION_FAILED;
            }

            // Now we can start setting up our render pass. Render passes are
            // configured using a RenderPassBeginInfo struct:

            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            // Pass in our renderPass
            renderPassInfo.renderPass = pGraphicsPipeline->renderPass;
            // Get the specific renderpass.
            renderPassInfo.framebuffer = pFramebuffers[bufferIndex];
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
            vkCmdBeginRenderPass(*buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            // Once the render pass has started, we can now attach the graphics pipeline. The second·
            // parameter of this function call specifies whether this pipeline object is a graphics·
            // or compute pipeline.
            vkCmdBindPipeline(*buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pGraphicsPipeline->graphicsPipeline);

            // Specify a list of vertex buffers that need to be recorded by the command buffer
            VkBuffer vertexBuffers[] = { vertexBuffer->bufferData.buffer };
            VkDeviceSize offsets[] = {0};

            // Bind the vertex buffers to the command buffer being recorded.
            vkCmdBindVertexBuffers(*buffer, 0, 1, vertexBuffers, offsets);

            vkCmdBindIndexBuffer(*buffer, indexBuffer->bufferData.buffer, 0,
                                 VK_INDEX_TYPE_UINT16);

            for (size_t j = 0; j < objectCount; j++) {

                vkCmdBindDescriptorSets(*buffer,VK_PIPELINE_BIND_POINT_GRAPHICS,
                    pGraphicsPipeline->pipelineLayout, 0, 1,
                    &descriptorSets[j][bufferIndex],0, nullptr);

                // Now we can tell Vulkan to draw our triangle. This function has the parameters:
                vkCmdDrawIndexed(*buffer, indexBuffer->indexCount, 1, 0, 0, 0);
            }
            // Now we can end the render pass:
            vkCmdEndRenderPass(*buffer);

            //  Now we can end the command buffer recording
            if (vkEndCommandBuffer(*buffer) != VK_SUCCESS) {
                return VK_ERROR_INITIALIZATION_FAILED;
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
        VkCommandBuffer* pCommandbuffers,
        VkDescriptorSetLayout* descriptorSetLayout,
        VkDescriptorPool* descriptorPool,
        VkDescriptorSet** descriptorSets,
        Buffers::BufferData** uniformBuffers,
        size_t objectCount
    ) {

        vkDeviceWaitIdle(deviceData->logicalDevice);

        cleanupSwapchain(deviceData->logicalDevice, pSwapchain, pGraphicsPipeline, 
            commandPool, pFramebuffers, pCommandbuffers, uniformBuffers, 
            *descriptorPool, objectCount);
        
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
            pSwapchain, descriptorSetLayout) != VK_SUCCESS) {
         
            return VK_ERROR_INITIALIZATION_FAILED;
        }
        // Re-create our framebuffers
        if (createFramebuffer(deviceData->logicalDevice, pFramebuffers,
            pSwapchain, pGraphicsPipeline) != VK_SUCCESS) {

            return VK_ERROR_INITIALIZATION_FAILED;
        }

        if (createDescriptorPool(deviceData->logicalDevice, pSwapchain->imageCount, 
            descriptorPool, objectCount) != VK_SUCCESS) {

            return VK_ERROR_INITIALIZATION_FAILED;
        }

        for (size_t i = 0; i < objectCount; i++) {

            if (createUniformBuffers(deviceData, uniformBuffers[i],
                pSwapchain->imageCount) != VK_SUCCESS) {

                return VK_ERROR_INITIALIZATION_FAILED;
            }

            if (createDescriptorSets(deviceData, descriptorSets[i], descriptorSetLayout,
                 descriptorPool, pSwapchain->imageCount, uniformBuffers[i])
                != VK_SUCCESS) {

                return VK_ERROR_INITIALIZATION_FAILED;
            }
        }

        // Re-create command buffers
        if (createCommandBuffers(deviceData->logicalDevice, pCommandbuffers,
            pGraphicsPipeline, pSwapchain, pFramebuffers, commandPool, 
            vertexBuffer, indexBuffer, descriptorSets, objectCount) 
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
        VkCommandBuffer* pCommandbuffers,
        Buffers::BufferData** uniformBuffers,
        VkDescriptorPool& descriptorPool,
        size_t objectCount) {

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

        for (size_t i = 0; i < objectCount; i++) {
            for (size_t j = 0; j < pSwapchain->imageCount; j++) {
                vkDestroyBuffer(device, uniformBuffers[i][j].buffer, nullptr);
                vkFreeMemory(device, uniformBuffers[i][j].bufferMemory, nullptr);
            }
        }

        vkDestroyDescriptorPool(device, descriptorPool, nullptr);
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

    VkResult createUniformBuffers(VulkanDeviceData* deviceData,
        Buffers::BufferData* uBuffers, uint32_t imageCount) {

        VkDeviceSize bufferSize = sizeof(Buffers::UniformBufferObject);

        for (size_t i = 0; i < imageCount; i++) {
            if (Buffers::createBuffer(
                deviceData->physicalDevice,
                deviceData->logicalDevice,
                bufferSize,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                uBuffers[i]) != VK_SUCCESS) {
                
                return VK_ERROR_INITIALIZATION_FAILED;
            }
        }
        
        return VK_SUCCESS;
    }

    VkResult createDescriptorSets(VulkanDeviceData* deviceData,
        VkDescriptorSet* sets, VkDescriptorSetLayout* layout, 
        VkDescriptorPool* pool, uint32_t imageCount, 
        Buffers::BufferData* uBuffers) {

        std::vector<VkDescriptorSetLayout> layouts(imageCount, *layout);

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = *pool;
        allocInfo.descriptorSetCount = imageCount;
        allocInfo.pSetLayouts = layouts.data();

        if (vkAllocateDescriptorSets(deviceData->logicalDevice, &allocInfo,
                                     sets) != VK_SUCCESS) {

            return VK_ERROR_INITIALIZATION_FAILED;
        }

        for (size_t i = 0; i < imageCount; i++) {
            
            VkDescriptorBufferInfo bufferInfo; 
            bufferInfo.buffer = uBuffers[i].buffer;
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(Buffers::UniformBufferObject);

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = sets[i];
            descriptorWrite.dstBinding = 0; 
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &bufferInfo;
            
            vkUpdateDescriptorSets(deviceData->logicalDevice, 1, &descriptorWrite, 
                0, nullptr);
        }

        return VK_SUCCESS;
    }
}
