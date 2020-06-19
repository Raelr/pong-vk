
workspace "Pong"
configurations { "Debug", "Release" }

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
     "$(VULKAN_SDK)/include",
     "vendor/glm",
     "vendor/GLFW/glfw/include",
}

filter "system:macosx"
    systemversion "latest"
    staticruntime "on"

    libdirs {
         "$(VULKAN_SDK)/lib**"
    }
    
    linkoptions { os.getenv("VULKAN_SDK") .. "lib/libvulkan.1.2.135.dylib" }

    links {
      "Cocoa.framework",
      "IOKit.framework",
      "CoreFoundation.framework",
    }

filter "configurations:Debug"
     defines { "DEBUG" }
     symbols "On"

filter "configurations:Release"
    defines { "RELEASE" }
    optimize "On"

