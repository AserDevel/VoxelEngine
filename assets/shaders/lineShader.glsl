#shader vertex
#version 430 core

layout (location = 0) in vec3 aPos; // Position of the vertex in world space

uniform mat4 matCamera;

void main() {
    gl_Position = matCamera * vec4(aPos, 1.0); // Transform to clip space
}

#shader fragment
#version 430 core

out vec4 FragColor;

uniform vec3 lineColor; // Line color as a uniform (RGB)

void main() {
    FragColor = vec4(lineColor, 1.0); // Final color with full alpha
}
