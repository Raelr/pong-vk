#version 450

// We can define a color which will be passed into the 
// fragment shader.
layout (location = 0) out vec3 fragColor;

// Since Vertex shaders in Vulkan are complex, we're  gonna 
// statically define them for now. 
vec2 positions[3] = vec2[](
    // Top of the triangle
    vec2(0.0, -0.5),
    // Bottom right of triangle
    vec2(0.5, 0.5),
    // Bottom left of triangle
    vec2(-0.5, 0.5)
);

// Since we want ever point to be be a different color, we
// define the RGB values for each point
vec3 colors[3] = vec3[] (
    // Red
    vec3(1.0, 0.0, 0.0),
    // Green
    vec3(0.0, 1.0, 0.0),
    // Blue
    vec3(0.0, 0.0, 1.0)
);

void main() {
    // Define the position of the triangle
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    // Pass the colors to the fragColor variable
    fragColor = colors[gl_VertexIndex];
}
