
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
     "GLFW"
}

includedirs { 
     "$(VULKAN_SDK)/include", 
     "vendor/glm",
     "vendor/GLFW/glfw/include"
}

filter "configurations:Debug"
     defines { "DEBUG" }
     symbols "On"

filter "configurations:Release"
    defines { "NDEBUG" }
    optimize "On"