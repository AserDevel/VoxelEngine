#shader vertex
#version 430 core

layout (location = 0) in vec3 position;
layout (location = 1) in uint materialID;   // 8-bits maps to material properties (color, transparency)
layout (location = 2) in uint metaData;     // 16 bits AO value, normal, skylight level and blocklight level

struct Material {
    vec4 color;
};

vec3 normals[6] = {
    vec3(1, 0, 0),  vec3(-1, 0, 0),  // +X, -X
    vec3(0, 1, 0),  vec3(0, -1, 0),  // +Y, -Y
    vec3(0, 0, 1),  vec3(0, 0, -1),  // +Z, -Z
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
    
    baseColor = materials[materialID].color;
    
    fragPos = position;
    
    normal = normals[metaData & 7];
    
    AOfactor = float((metaData >> 3) & 3);
    blockLightFactor = float((metaData >> 5) & 15) / 15.0;
    skyLightFactor = float((metaData >> 9) & 15) / 15.0;
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