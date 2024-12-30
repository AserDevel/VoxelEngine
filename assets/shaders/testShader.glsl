#shader vertex
#version 430 core

layout (location = 0) in vec3 position;
//layout (location = 1) in uint data;

uniform mat4 matCamera;

void main() {
    gl_Position = matCamera * vec4(position, 1.0);
}

#shader fragment
#version 430 core

out vec4 fragColor;

void main() {
    fragColor = vec4(1, 1, 1, 1);
}