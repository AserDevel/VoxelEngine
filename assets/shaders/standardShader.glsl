#shader vertex
#version 430 core

layout (location = 0) in vec3 position;
layout (location = 1) in uint data;

struct Material {
    vec4 color;
};

vec3 normals[6] = {
    vec3(1, 0, 0),  vec3(-1, 0, 0),  // +X, -X
    vec3(0, 0, 1),  vec3(0, 0, -1),  // +Z, -Z
    vec3(0, 1, 0),  vec3(0, -1, 0),  // +Y, -Y
};

out vec4 baseColor;
out vec3 fragPos;
out vec3 normal;
flat out uint skyLightLevel;
flat out uint blockLightLevel;
out float AOfactor;

uniform mat4 matCamera;
uniform Material materials[256];

void main() {
    gl_Position = matCamera * vec4(position, 1.0);

    skyLightLevel = data & 15;
    blockLightLevel = (data >> 4) & 15;
    AOfactor = float((data >> 8) & 3);
    
    normal = normals[(data >> 10) & 7];
    uint materialID = (data >> 13) & 127;
    
    baseColor = materials[materialID].color;
    
    fragPos = position;
}


#shader fragment
#version 430 core

in vec4 baseColor;
in vec3 fragPos;
in vec3 normal;
flat in uint skyLightLevel;
flat in uint blockLightLevel;
in float AOfactor;

out vec4 fragColor;

void main() {
    float ambient = 0.1f * AOfactor;
    float skyLight = skyLightLevel / 16.0f;

    fragColor = baseColor * (ambient + skyLight);
}