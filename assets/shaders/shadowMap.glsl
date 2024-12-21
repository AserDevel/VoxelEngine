#shader vertex
#version 460 core

layout(location = 0) in vec3 position;

uniform mat4 lightSpaceMatrix; // Light's projection-view matrix

void main() {
    gl_Position = lightSpaceMatrix * vec4(position, 1.0);
}

#shader fragment
#version 460 core

void main() {
    // no output needed
}