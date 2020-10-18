#include "texture2d.h"

namespace Renderer {

    void destroyTexture2D(VkDevice device, Texture2D& texture) {

        vkDestroyImage(device, texture.image, nullptr);
        vkFreeMemory(device, texture.memory, nullptr);
    }

}

