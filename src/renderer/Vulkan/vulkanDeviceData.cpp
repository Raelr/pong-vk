#include "vulkanDeviceData.h"

namespace Renderer {

    Status checkValidationLayerSupport(
            uint32_t layerCount,
            VkLayerProperties* layerProperties,
            const char** pValidationLayers,
            uint32_t pValidationLayersCount
    ) {
        // Get the layer names themselves USING the count variable.
        vkEnumerateInstanceLayerProperties(&layerCount, layerProperties);
        uint8_t foundCount = 0;

        // Search to see whether the validation layers requested are supported by Vulkan.
        for (size_t i = 0; i < pValidationLayersCount; i++) {
            for (size_t j = 0; j < layerCount; j++) {
                if (strcmp(pValidationLayers[i], layerProperties[j].layerName) == 0) {
                    foundCount++;
                }
            }
        }

        // Throw an error and end the program if the validation layers aren't found.
        if (foundCount != pValidationLayersCount) {
            return Status::FAILURE;
        }

        INFO("Requested Validation layers exist!");

        return Status::SUCCESS;
    }
}