#ifndef PONG_VK_RENDERER2D_H
#define PONG_VK_RENDERER2D_H

#include "../vulkanUtils.h"

namespace Renderer2D {

    struct Renderer2DData {
        VulkanUtils::GraphicsPipelineData graphicsPipeline{nullptr};
    };


}

#endif //PONG_VK_RENDERER2D_H
