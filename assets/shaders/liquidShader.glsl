#shader vertex
#version 430 core

layout (location = 0) in vec3 position;

uniform mat4 matCamera;

void main() {
    gl_Position = matCamera * vec4(position, 1.0);
}

#shader fragment
#version 430 core

out vec4 fragColor;

void main() {
    fragColor = vec4(0.0, 0.0, 1.0, 0.5);
}
