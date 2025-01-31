#version 460 core

#include "assets/shaders/rayTracing.glsl"

layout (location = 0) out vec4 globalLight;
layout (location = 1) out vec4 specialLight;

// SkyLight uniforms
uniform vec3 skyLightDir;
uniform vec3 skyLightColor;

// Frame uniforms
uniform bool cameraMoved;
uniform vec2 randomOffset;
uniform vec2 screenSize;
uniform sampler2D positionTex;
uniform sampler2D normalTex;
uniform sampler2D blueNoiseTex;

float rndCount = 0;

vec3 random() {
    vec2 offset = randomOffset + (randomOffset * rndCount);
    vec2 uv = (gl_FragCoord.xy + offset) / vec2(256, 256);

    vec3 random = texture(blueNoiseTex, uv).xyz;
    rndCount += 1.0;
    return random;
}

// creates a random sphere based on normal and a seed vector
vec3 randomSphereSample(vec3 normal) {
    vec3 offset = 2 * random() - vec3(1.0); // random vector normalized to [-1, 1]
    vec3 dir = normalize(normal + offset);
    return dir;
}

vec3 computeGlobalIllumination(vec3 origin, vec3 normal, int samples) {
    vec3 accumulatedLight = vec3(0.0);

    for (int i = 0; i < samples; i++) {
        vec3 dir = randomSphereSample(normal);
        RayData ray = traceRay(origin, dir, 16);
        if (!ray.hit) {
            accumulatedLight += skyLightColor * max(dot(dir, -skyLightDir), 0.5);
        }
    }
    
    return accumulatedLight / float(samples);
}

float computeSoftShadow(vec3 rayOrigin, int numSamples) {
    float jitterAmount = 0.2;  // Amount of jitter around the light direction
    float occlusion = 0.0;
    
    for (int i = 0; i < numSamples; i++) {
        vec3 sampleDir = randomSphereSample(-skyLightDir);

        // Rotate sampleDir along with lightDir
        sampleDir = normalize(-skyLightDir + jitterAmount * sampleDir);

        // Use sample for soft shadows
        RayData ray = traceRay(rayOrigin, sampleDir, 32);
        if (ray.hit) {
            occlusion += 1.0;
        } 
    }
    occlusion /= float(numSamples); // map to [0.0, 1.0] where 1.0 is fully occluded

    return occlusion; 
}

void main() {
    vec2 uv = gl_FragCoord.xy / screenSize;
    vec3 position = texture(positionTex, uv).xyz;
    vec3 normal = texture(normalTex, uv).xyz;

    int samples = 2;
    if (cameraMoved) samples = 4;

    vec3 light = computeGlobalIllumination(position, normal, samples);
    
    // Compute soft shadow if the surface is exposed to the sky light
    float shadow = 1.0;
    if (dot(normal, -skyLightDir) > 0.0) {
        shadow = computeSoftShadow(position, samples); 
    }
    light *= 0.5 + 0.5 * (1.0 - shadow);
    
    globalLight = vec4(light, 1.0);
}