// buffer containing all active voxels
layout(std430, binding = 0) buffer voxelBuffer {
    uint voxels[];
};

// buffer mapping chunk position to index by flattening the 3D array
layout(std430, binding = 1) buffer chunkOffsetBuffer {
    int chunkOffsets[];
};

uniform int worldChunkLen;

// Convert voxel position to a voxel
uint positionToVoxel(vec3 voxelPos) {
    uvec3 chunkPos = uvec3(voxelPos) / 16;
    uint offsetIdx = chunkPos.x + (chunkPos.y * worldChunkLen) + (chunkPos.z * worldChunkLen * worldChunkLen);
    int offset = chunkOffsets[offsetIdx];
    if (offset == -1) {
        return 0;
    } else {
        uvec3 localPos = uvec3(voxelPos) - (chunkPos * 16);
        uint idx = offset + localPos.x + (localPos.y * 16) + (localPos.z * 16 * 16);
        return voxels[idx];
    }
}

struct RayData {
    bool hit;
    vec3 hitPos;
    vec3 normal;
    uint voxel;
};

// Trace ray through the world using origin and direction
RayData traceRay(vec3 rayOrigin, vec3 rayDir, uint maxSteps) {
    uint worldLen = worldChunkLen * 16;
    vec3 step = sign(rayDir);
    vec3 voxelPos = floor(rayOrigin);
    vec3 tDelta = abs(1.0 / rayDir);
    vec3 tMax;
    for (int i = 0; i < 3; i++) {
        if (rayDir[i] < 0) {
            tMax[i] = (rayOrigin[i] - voxelPos[i]) * tDelta[i]; 
        } else {
            tMax[i] = (voxelPos[i] + 1 - rayOrigin[i]) * tDelta[i];
        }
    }
    for (int i = 0; i < maxSteps; i++) {
        uint voxel = positionToVoxel(voxelPos);
        if (voxel != 0) {
            float epsilon = 0.0001;
            tMax -= tDelta;
            vec3 hitPos = rayOrigin;
            vec3 normal = vec3(0.0);
            if (tMax.x >= tMax.y && tMax.x >= tMax.z) {
                normal = vec3(-step.x, 0.0, 0.0); // X-axis normal
                hitPos += rayDir * tMax.x + normal * epsilon;
            } else if (tMax.y >= tMax.z) {
                normal = vec3(0.0, -step.y, 0.0); // Y-axis normal
                hitPos += rayDir * tMax.y + normal * epsilon;
            } else {
                normal = vec3(0.0, 0.0, -step.z); // Z-axis normal
                hitPos += rayDir * tMax.z + normal * epsilon;
            }

            RayData data = {true, hitPos, normal, voxel};
            return data;
        }

        // Move to the next voxel in the direction of the smallest distance
        if (tMax.x < tMax.y && tMax.x < tMax.z) {
            tMax.x += tDelta.x; // Move along X
            voxelPos.x += step.x;
        } else if (tMax.y < tMax.z) {
            tMax.y += tDelta.y; // Move along Y
            voxelPos.y += step.y;
        } else {
            tMax.z += tDelta.z; // Move along Z
            voxelPos.z += step.z;
        }

        if (voxelPos.x < 0 || voxelPos.x >= worldLen ||
            voxelPos.y < 0 || voxelPos.y >= worldLen || 
            voxelPos.z < 0 || voxelPos.z >= worldLen) {
            break;
        } 
    }    

    RayData data = {false, vec3(0.0), vec3(0.0), 0};
    return data;
}
