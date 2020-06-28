#!/bin/bash

$VULKAN_SDK/bin/glslangValidator -V src/shaders/vert.vert && mv vert.spv src/shaders
$VULKAN_SDK/bin/glslangValidator -V src/shaders/frag.frag && mv frag.spv src/shaders
