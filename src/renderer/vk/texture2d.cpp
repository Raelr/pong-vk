#include "texture2d.h"

namespace Renderer {

    void destroyTexture2D(VkDevice device, Texture2D& texture) {

        vkDestroyImageView(device, texture.view, nullptr);
        vkDestroyImage(device, texture.image, nullptr);
        vkFreeMemory(device, texture.memory, nullptr);
    }

}

