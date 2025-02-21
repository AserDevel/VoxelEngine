#version 460 core

#include "assets/shaders/rayTracing.glsl"

layout (location = 0) out vec4 lighting;

// SkyLight uniforms
uniform vec3 skyLightDir;
uniform vec3 skyLightColor;

// Frame uniforms
uniform bool cameraMoved;
uniform vec3 eyePos;
uniform vec2 randomOffset;
uniform vec2 screenSize;
uniform sampler2D positionTex;
uniform sampler2D normalTex;
uniform isampler2D voxelTex;
uniform sampler2D blueNoiseTex;

struct Material {
    vec4 color;
    float specularity;
};

uniform Material[256] materials;

float rndCount = 0;

// generates a random blue noise vec3
vec3 random() {
    vec2 offset = randomOffset + (randomOffset * rndCount);
    vec2 uv = (gl_FragCoord.xy + offset) / vec2(256, 256);

    vec3 random = texture(blueNoiseTex, uv).xyz;
    rndCount += 1.0;
    return random;
}

// creates a random sphere based on the normal and a seed vector
vec3 randomSphereSample(vec3 normal) {
    vec3 offset = 2 * random() - vec3(1.0); // random vector normalized to [-1, 1]
    vec3 dir = normalize(normal + offset);
    return dir;
}

// global illumination and ambient occlusion
vec3 computeGlobalIllumination(vec3 origin, vec3 normal, int samples) {
    vec3 accumulatedLight = vec3(0.0);

    for (int i = 0; i < samples; i++) {
        vec3 dir = randomSphereSample(normal);
        RayData ray = traceRay(origin, dir, 16, false);
        if (!ray.hit) {
            accumulatedLight += skyLightColor * max(dot(dir, skyLightDir), 0.5);
        } else {
            // add ambient occlussion to the edges of the voxel by using the ray's travel distance
            float globalAmbience = 0.2;
            float distance = length(ray.hitPos - origin);
            float ambience = min(globalAmbience * distance, globalAmbience);
            accumulatedLight += vec3(1.0) * ambience;
        }
    }
    
    return accumulatedLight / float(samples);
}

// direct soft shadows
float computeSoftShadow(vec3 rayOrigin, int numSamples) {
    float jitterAmount = 0.1;  // Amount of jitter around the light direction
    float occlusion = 0.0;
    
    for (int i = 0; i < numSamples; i++) {
        vec3 sampleDir = randomSphereSample(skyLightDir);

        // Rotate sampleDir along with lightDir
        sampleDir = normalize(skyLightDir + jitterAmount * sampleDir);

        // Use sample for soft shadows
        RayData ray = traceRay(rayOrigin, sampleDir, 64, false);
        if (ray.hit) {
            occlusion += 1.0;
        } 
    }
    occlusion /= float(numSamples); // map to [0.0, 1.0] where 1.0 is fully occluded

    return occlusion; 
}

// reflections and specular lighting
vec3 computeReflection(vec3 rayOrigin, vec3 reflectionDir, float specularity, int numSamples) {
    float jitterAmount = (1.0 - specularity);
    vec3 reflection = vec3(0.0);
    
    for (int i = 0; i < numSamples; i++) {
        vec3 sampleDir = randomSphereSample(reflectionDir);

        // Rotate sampleDir along with reflection dir based on specularity
        sampleDir = normalize(reflectionDir + jitterAmount * sampleDir);

        // Use sample for reflection
        RayData ray = traceRay(rayOrigin, sampleDir, 64, false);
        if (ray.hit) {
            // add reflection from material color;
            reflection += materials[ray.voxel].color.rgb;
        } else {
            // Add specular skylight on reflective material
            reflection += skyLightColor + specularity * skyLightColor * pow(max(dot(skyLightDir, sampleDir), 0.0), 32); 
        }
    }
    reflection /= float(numSamples); // map to [0.0, 1.0] where 1.0 is fully occluded

    return reflection; 
}

void main() {
    vec2 uv = gl_FragCoord.xy / screenSize;
    vec3 position = texture(positionTex, uv).xyz;
    vec3 normal = texture(normalTex, uv).xyz;
    uint voxel = texture(voxelTex, uv).r;

    // handle no hit rays
    if (position == vec3(0.0)) {
        lighting = vec4(0.0);
        return;
    }

    // more samples on dynamic frames
    int numSamples = 1;
    if (cameraMoved) numSamples = 4;

    // compute global illumination
    vec3 light = computeGlobalIllumination(position, normal, numSamples);
    
    // Compute soft shadow if the surface is exposed to the sky light
    float shadow = 0.0;
    if (dot(normal, skyLightDir) > 0.0) {
        shadow = computeSoftShadow(position, numSamples);
        shadow = 1.0 - shadow;
    }
    light *= 0.5 + 0.5 * shadow;

    // add reflections and specular lighting
    float spec = materials[voxel].specularity;
    if (spec != 0.0) {
        vec3 eyeDir = normalize(position - eyePos);
        vec3 reflectionDir = normalize(eyeDir - 2.0 * dot(eyeDir, normal) * normal);
        vec3 reflection = computeReflection(position, reflectionDir, spec, numSamples);
        light = mix(light, reflection, 0.5);
    }

    // handle transparency
    /*
    vec4 color = materials[voxel].color;
    if (color.a < 1.0) {
        vec3 rayDir = normalize(position - eyePos);
        RayData ray = traceRay(position, rayDir, 64, true);
        if (ray.hit) {
            float mixer = 1.0 - color.a * (length(ray.hitPos - position) / 64.0);
            light = mix(light, materials[ray.voxel].color.rgb, mixer);
        }
    }
    */

    lighting = vec4(light, 1.0);
}