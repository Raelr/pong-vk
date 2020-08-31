#ifndef PONG_VK_VALIDATIONLAYERS_H
#define PONG_VK_VALIDATIONLAYERS_H

#include "../../logger.h"
#include <vulkan/vulkan.h>

namespace Renderer {

    // A debug function callback - where the message data actually goes when triggered by the
    // validation layer.
    // This function uses three macros to define it's signature (used to help Vulkan know that
    // this method is valid).
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            [[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,     // The severity of the message
            [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageType,                // The actual TYPE of message
            [[maybe_unused]] const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,  // Struct with data of the message
            [[maybe_unused]] void* pUserData) {                                          // User defined data (optional)

        // In this case we simply print out the message itself to the console.
        ERROR(pCallbackData->pMessage);

        // Return value determines whether the call that triggers this message should be aborted.
        // Generally these should return false.
        return VK_FALSE;
    }

    // A proxy function which calls the vkCreateDebugUtilsMessengerEXT function.
    // The reason we need this function is because 'vkCreateDebugUtilsMessengerEXT' is an
    // extension function - meaning that it isn't automatically loaded into memory. As such,
    // we need to manually look up it's memory and call it from there. Vulkan provides us
    // with a utility function: 'vkGetInstanceProcAddr' for this exact purpose.
    static VkResult createDebugUtilsMessengerEXT(
            VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
            const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {

        // Store the function in a variable 'func'.
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
                instance, "vkCreateDebugUtilsMessengerEXT");

        // Make sure we got the correct function.
        if (func != nullptr) {
            // Call the function
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        } else {
            // Return an error
            return VkResult::VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    // Simply accepts a reference to a messenger config struct and fills it with data.
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& messengerInfo) {
        messengerInfo = {};
        // Specify the type of struct that's being populated
        messengerInfo.sType =           VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        // You can specify the severity levels of logs that this struct will accept.
        messengerInfo.messageSeverity = (unsigned) VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                                        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                                        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        // You can also limit the actual message type to specific types.
        messengerInfo.messageType =     (unsigned) VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                                        | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                                        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        // Finally, you can store the actual debug callback in the struct
        messengerInfo.pfnUserCallback = debugCallback;
        messengerInfo.pUserData =       nullptr;
    }

    // Similar to messenger creation - destroying the messenger also requires the calling of
    // an unloaded message. We need to do the same thing as before and load the method from memory.
    // In this case we want to get a method for cleaning up messenger memory.
    static void destroyDebugUtilsMessengerEXT(VkInstance instance,
                                              VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
        // Load the method from memory
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
                instance, "vkDestroyDebugUtilsMessengerEXT");

        if (func != nullptr) {
            // Call the method if it was returned successfully.
            func(instance, debugMessenger, pAllocator);
        }
    }
}

#endif //PONG_VK_VALIDATIONLAYERS_H
