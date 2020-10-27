#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;
// Define a variable for the color of each vertex
layout(location = 0) out vec4 outColor;
// Define the variable that will be passed in (from 
// the vertex shader)
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

void main() {
    // Define the color as being that of the fragColor
    // variable
    outColor = vec4(fragTexCoord, 0.0, 1.0);
}
