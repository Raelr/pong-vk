#ifndef PONG_VK_INITIALISERS_H
#define PONG_VK_INITIALISERS_H

#include <vulkan/vulkan.h>

namespace Renderer {

    VkApplicationInfo createVulkanApplicationInfo(const char*, const char*, uint32_t, uint32_t, uint32_t);

}


#endif //PONG_VK_INITIALISERS_H
