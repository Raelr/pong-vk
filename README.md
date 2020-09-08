# pong-vk

## Overview

An implmentation of Pong using the Vulkan rendering API. This an educational project intended to be used to learn the basics of Vulkan. Since Pong is a fairly straighforward game, it was chosen to be the implementation for the project. 

This repo will primarily follow the Vulkan tutorials by [Alexander Overvoorde](https://vulkan-tutorial.com/en/Introduction). These will be used to build the renderer. All game logic will be handled seprately of the tutorial. 

## Progress

So far, we've succeeded in getting multiple objects rendering to screen.

<img src="https://github.com/Raelr/pong-vk/blob/master/assets/mulipleObjects.png" alt="Quads" width="400" height="300">

We've built a simple API for rendering objects to the screen. The renderer currently allocates a new uniform buffer and descriptor set for all new objects on the screen. The API currently works as follows:

```c++
Renderer::Renderer renderer;

    // Let the renderer know that we want to load in default validation layers
    Renderer::loadDefaultValidationLayers(&renderer);
    Renderer::loadDefaultDeviceExtensions(&renderer);

    if (Renderer::initialiseRenderer(&renderer, enableValidationLayers, window)
        != Renderer::Status::SUCCESS) {
        PONG_FATAL_ERROR("Failed to initialise renderer!");
    }

    // ------------------------ SCENE SETUP -----------------------------
    
    // Player Definition
    
    PlayerData players[64];
    uint32_t currentPlayers = 1;

    players[0] = {
        {-250.0f,-150.0f,1.0f},
        {0.0f,0.0f,1.0f},
        {200.0f,200.0f, 0.0f}, -90.0f, 0
    };
    
    [...]
    
    // Main Loop
    
    for (int i = 0; i < currentPlayers; i++) {
        PlayerData& player = players[i];
        Renderer::drawQuad(&renderer, player.position, player.rotation,
            getTime() * glm::radians(player.rotationAngle), player.scale, player.playerIndex);
    }

    if (Renderer::drawFrame(&renderer, &pongData.framebufferResized, window)
        == Renderer::Status::FAILURE) {
        ERROR("Error drawing frame - exiting main loop!");
        break;
    }
    
    [...]
    
    // Shutdown
    
    Renderer::cleanupRenderer(&renderer, enableValidationLayers);
```
Currently, the renderer relies on all objects being registered prior to the main loop. The next step is to implement a dynamic uniform buffer for uploading data to the GPU. This should allow us to allocate data dynamically so that scenes can be uploaded and flushed per frame. 

## Setup Notes

**NOTE:** This prject contains submodules. Please remember to clone the project with --recursive-submodules prepended. I.e:

```
$ git clone --recurse-submodules https://github.com/Raelr/pong-vk.git
```

Alternatively, you can also clone the repo and run the commands:

```
$ git submodules init
$ git submodules update
```

If you find that the submodules fail to install, you should try and delete the following directories:

* vendor/glm
* vendor/spdlog
* vendor/GLFW/glfw

Then try and re-run the submodules command and see if it works.

### Required Compiler

Please compile the project using [Clang](https://clang.llvm.org/). 

## MacOS Setup

To successfully run the project in MacOS, a few conditions must be met: 

1. The MacOS Vulkan SDK must be installed on the machine (SDK can be found [here](https://vulkan.lunarg.com/))
2. The VULKAN_SDK environment variable must be set on your system. This can be done by exporting the variable in your `.bash_profile` (or .zshrc) file. Steps to doing that are as follows:

   1. Open your .bash_profile or .zshrc file using the command: `~/.bash_profile` or `~/.zshrc`. 
   2. Add an export command to the file such as: `export VULKAN_SDK=<PATH_TO_VULKAN_SDK_MACOS_DIRECTORY>`.
      Your VulkanSDK development tree should have a MacOS directory, this is where the variable should point to.
   3. Save the file, close the terminal and run the following command: `echo $VULKAN_SDK`. If you see output then you've successfully configured the environment variable!
   
### Building the project

This project primarily uses [Premake](https://github.com/premake/premake-core) as it's build system. Premake is a tool which essentually preprocesses a project for usage by other tools. Premake can generate project files for Visual Studio, XCode, Gmake, and others (Please check [here](https://github.com/premake/premake-core/wiki/Using-Premake) for available configurations).  

For this example we'll use gmake2 (since this is the system used by this project).

First, generate the files using the `premake` command:

```
$ premake/premake5 gmake 
```

You should see output similar to the following:

```
Building configurations...
Running action 'gmake'...
Done (58ms).
```

Once this is done, simply run the build commands for Make:

```
$ make config=debug all
```

You should then see similar output to this:

```
==== Building GLFW (debug) ====
==== Building Pong (debug) ====
main.cpp
Linking Pong
```

Finally, run the project by running the command:

```
$ ./bin/Debug/Pong 
```

You should then see the window show up! 

You can also simply navigate to the `bin/Debug` folder and run the `Pong` file manually from there. 

## Windows Setup

Windows is now supported. Like the MacOS installation, it requires that you have the Vulkan installation directory set in your environment variables.

You should be able to generate your Visual Studio project code by opening the windows CLI and navigating to the project directory. When in the project directory, run the following command: 

```
[From project directory]
premake\premake5.exe vs2019
```

This should generate the appropriate visual studio files. After that, open the generated .sln file and you should be able to run the project from there!

**NOTE:** Remember to change your Visual Studio compiler to Clang. You can do so by following the instructions laid out [here](https://docs.microsoft.com/en-us/cpp/build/clang-support-msbuild?view=vs-2019). 
