
workspace "Pong"
     startproject "Pong"
     configurations { 
          "DEBUG", 
          "RELEASE" 
     }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
include "vendor/GLFW/"

project "Pong"
     kind "ConsoleApp"
     language "C++"
     cppdialect "C++17"
     targetdir "bin/%{cfg.buildcfg}"

     files { "src/**.h", "src/**.cpp" }

     links {
          "GLFW",
     }

     includedirs { 
          os.getenv("VULKAN_SDK") .. "/include",
          "vendor/glm",
          "vendor/GLFW/glfw/include",
          "vendor/spdlog/include",
          "vendor/stb/"
     }

     filter "system:macosx"
          architecture "x64"
          systemversion "latest"
          staticruntime "on"
          linkoptions { os.getenv("VULKAN_SDK") .. "lib/libvulkan.1.2.135.dylib" }
          defines { 
               "VK_ICD_FILENAMES=" .. os.getenv("VULKAN_SDK") .. "share/vulkan/icd.d/MoltenVK_icd.json",
               "VK_LAYER_PATH=" .. os.getenv("VULKAN_SDK") .. "share/vulkan/explicit_layer.d"
          }
          links {
               "Cocoa.framework",
               "IOKit.framework",
               "CoreFoundation.framework",
          }

     filter "system:windows"
          architecture "x86"
          systemversion "latest"
          staticruntime "on"
          links {
               os.getenv("VULKAN_SDK") .. "/lib32/vulkan-1.lib",
          }

     filter "configurations:Debug"
          defines { "DEBUG" }
          symbols "On"

     filter "configurations:Release"
          defines { "RELEASE" }
          optimize "On"