#version 450

// Layout the struct for handling a model-view projection
layout (binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout (location = 0) in vec2 inPosition;
layout (location = 1) in vec3 inColor;

// We can define a color which will be passed into the 
// fragment shader.
layout (location = 0) out vec3 fragColor;

void main() {
    // Define the position of the triangle
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 0.0, 1.0);
    // Pass the colors to the fragColor variable
    fragColor = inColor;
}
