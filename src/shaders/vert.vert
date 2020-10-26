#version 450
#extension GL_ARB_separate_shader_objects : enable

// Layout the struct for handling a model-view projection
layout (binding = 1) uniform UniformBufferObject {
    mat4 mvp;
    vec3 color;
} ubo;

layout (location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoord;

// We can define a color which will be passed into the 
// fragment shader.
layout (location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    // Define the position of the triangle
    gl_Position = ubo.mvp * vec4(inPosition, 0.0, 1.0);
    // Pass the colors to the fragColor variable
    fragColor = ubo.color;
    fragTexCoord = inTexCoord;
}
