#include "initialisers.h"

namespace Renderer {
    VkApplicationInfo createVulkanApplicationInfo(const char* pApplicationName, const char* pEngineName,
        uint32_t applicationVersion, uint32_t engineVersion, uint32_t apiVersion) {

        // Define the configuration details of the vulkan application.
        VkApplicationInfo appInfo {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = pApplicationName;
        appInfo.applicationVersion = applicationVersion;
        appInfo.pEngineName = pEngineName;
        appInfo.engineVersion = engineVersion;
        appInfo.apiVersion = apiVersion;

        return appInfo;
    }
}