#include "texture2d.h"

namespace Renderer {

    void destroyTexture2D(VkDevice device, Texture2D& texture) {

        vkDestroySampler(device, texture.sampler, nullptr);
        vkDestroyImageView(device, texture.view, nullptr);
        vkDestroyImage(device, texture.image, nullptr);
        vkFreeMemory(device, texture.memory, nullptr);
    }

}

