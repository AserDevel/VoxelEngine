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
out float skyLightFactor;
out float blockLightFactor;
out float AOfactor;

uniform mat4 matCamera;
uniform Material materials[256];

void main() {
    gl_Position = matCamera * vec4(position, 1.0);

    skyLightFactor = float(data & 15) / 15.0;
    blockLightFactor = float((data >> 4) & 15) / 15.0;
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
in float skyLightFactor;
in float blockLightFactor;
in float AOfactor;

out vec4 fragColor;

void main() {
    vec3 ambient = 0.1f * AOfactor * vec3(1, 1, 1);
    vec3 skyLight = skyLightFactor * vec3(1, 0.9, 0.9);

    fragColor = baseColor * (vec4(ambient, 1.0) + vec4(skyLight, 1.0));
}