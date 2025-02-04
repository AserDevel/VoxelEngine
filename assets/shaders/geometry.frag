#version 460 core

#include "assets/shaders/rayTracing.glsl"

// Outputs
layout(location = 0) out vec3 position;
layout(location = 1) out vec3 normal;
layout(location = 2) out uint voxel;

// Camera uniforms
uniform mat4 invViewProj;
uniform vec3 localCamPos;
uniform vec2 screenSize;

// world uniforms
uniform vec3 worldBasePos;

void main() {
    vec3 rayOrigin = localCamPos;

    // Convert screen-space to Normalized Device Coordinates (NDC)
    vec2 ndc = (gl_FragCoord.xy / screenSize) * 2.0 - 1.0; // Map to range [-1, 1]

    // NDC position
    vec4 ndcPos = vec4(ndc, 1.0, 1.0); // Homogeneous clip space

    // Transform from NDC to world space using the inverse view-projection matrix
    vec4 worldPos = invViewProj * ndcPos;
    worldPos /= worldPos.w; // Perform perspective divide

    vec3 rayDir = normalize(worldPos.xyz);

    RayData cameraRay = traceRay(rayOrigin, rayDir, 512);
    if (cameraRay.hit) {
        position = cameraRay.hitPos;
        normal = cameraRay.normal;
        voxel = cameraRay.voxel;
    } else {
        position = vec3(0.0);
        normal = vec3(0.0);
        voxel = 0;
    }
}