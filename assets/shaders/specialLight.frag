#version 460 core

#include "assets/shaders/rayTracing.glsl"

layout (location = 0) out vec4 globalLight;
layout (location = 1) out vec4 specialLight;

// SkyLight uniforms
uniform vec3 skyLightDir;
uniform vec3 skyLightColor;

// Frame uniforms
uniform vec3 eyePos;
uniform vec2 screenSize;
uniform sampler2D positionTex;
uniform sampler2D normalTex;
uniform isampler2D voxelTex;

struct Material {
    vec4 color;
    float specularity;
};

uniform Material[256] materials;

vec3 computeReflection(vec3 dir, vec3 normal) {
    return normalize(dir - 2.0 * dot(dir, normal) * normal);
}

void main() {
    vec2 uv = gl_FragCoord.xy / screenSize;
    vec3 position = texture(positionTex, uv).xyz;
    vec3 normal = texture(normalTex, uv).xyz;
    uint voxel = texture(voxelTex, uv).r;

    // handle no hit rays
    if (position == vec3(0.0)) {
        specialLight = vec4(0.0);
        return;
    }

    vec3 finalLight = vec3(0.0);

    if (materials[voxel].specularity != 0.0) {
        vec3 reflectionDir = computeReflection(normalize(position - eyePos), normal);
        RayData reflectionRay = traceRay(position, reflectionDir, 64);
        if (reflectionRay.hit) {
            vec3 reflectedColor = materials[reflectionRay.voxel].color.rgb;
            finalLight = 0.2 * reflectedColor;
        } else {
            finalLight = 0.2 * skyLightColor;
        }
    }

    specialLight = vec4(finalLight, 1.0);
}