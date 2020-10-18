#ifndef PONG_VK_TEXTURE2D_H
#define PONG_VK_TEXTURE2D_H

#include <vulkan/vulkan.h>
#include "../core.h"

namespace Renderer {

    struct Texture2D {
        VkImage image               {VK_NULL_HANDLE};
        VkDeviceMemory memory       {VK_NULL_HANDLE};
        VkImageLayout layout        {};
    };

    void destroyTexture2D(VkDevice, Texture2D&);
}

#endif //PONG_VK_TEXTURE2D_H
