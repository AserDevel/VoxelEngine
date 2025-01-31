#shader vertex
#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;

out vec2 TexCoords;

void main()
{
    TexCoords = texCoord;
    gl_Position = vec4(position, 1.0);
}

#shader fragment
#version 460 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D shadowMap;

void main()
{
    // Sample the shadow map texture to display depth values
    float depth = texture(shadowMap, TexCoords).r;
    FragColor = vec4(vec3(depth), 1.0); // Show depth in grayscale
}
