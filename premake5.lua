workspace "Pong"
    configurations { "Debug", "Release" }

project "Pong"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    targetdir "bin/%{cfg.buildcfg}"

    files { "src/**.h", "src/**.cpp" }
    links { "$(VULKAN_SDK)/lib/vulkan-1.lib" }
    includedirs { "$(VULKAN_SDK)/include" }

    filter "configurations:Debug"
         defines { "DEBUG" }
         symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"