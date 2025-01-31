#version 460 core

#include "assets/shaders/rayTracing.glsl"

layout (location = 0) out vec4 globalLight;
layout (location = 1) out vec4 specialLight;

// SkyLight uniforms
uniform vec3 skyLightDir;
uniform vec3 skyLightColor;

// Frame uniforms
uniform mat4 matViewProj;
uniform vec3 eyePos;
uniform vec2 screenSize;
uniform sampler2D positionTex;
uniform sampler2D normalTex;
uniform sampler2D globalLightTex;

vec2 computeProjectionCoords(vec3 position) {
    // World position from current frame
    vec4 worldPos = vec4(position, 1.0);

    // Convert back to previous clip space
    vec4 prevClipPos = matViewProj * worldPos;

    // Perform perspective divide to normalize coordinates
    prevClipPos /= prevClipPos.w;

    // Convert NDC [-1, 1] back to UV [0, 1]
    vec2 uvProjected = 0.5 + 0.5 * prevClipPos.xy;

    // Clamp to valid UV range
    uvProjected = clamp(uvProjected, vec2(0.0), vec2(1.0));

    return uvProjected;
}

vec3 computeReflection(vec3 dir, vec3 normal) {
    return normalize(dir - 2.0 * dot(dir, normal) * normal);
}

void main() {
    vec2 uv = gl_FragCoord.xy / screenSize;
    vec3 position = texture(positionTex, uv).xyz;
    vec3 normal = texture(normalTex, uv).xyz;

    vec3 finalLight = vec3(0.0);
    vec3 reflectionDir = computeReflection(normalize(position - eyePos), normal);
    RayData reflectionRay = traceRay(position, reflectionDir, 32);
    if (reflectionRay.hit) {
        vec2 uv = computeProjectionCoords(reflectionRay.hitPos);
        vec3 baseColor = vec3(0.3, 1.0, 0.3);
        finalLight = baseColor * texture(globalLightTex, uv).rgb;
    }

    specialLight = vec4(finalLight, 1.0);
}