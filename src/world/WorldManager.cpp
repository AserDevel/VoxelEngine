#include "world/WorldManager.h"
#include <queue>

Vec3 WorldManager::worldToChunkPosition(const Vec3& worldPosition) const {
    return floor(worldPosition / chunkSize);
}

Chunk* WorldManager::addChunk(const Vec3& chunkPosition, int bufferOffset) {
    Vec3 worldPosition = chunkPosition * chunkSize;
    chunkCache[chunkPosition] = std::make_unique<Chunk>(worldPosition, bufferOffset);
    return chunkCache[chunkPosition].get();
}

Chunk* WorldManager::getChunk(const Vec3& chunkPosition) const {
    auto it = chunkCache.find(chunkPosition);
    if (it != chunkCache.end()) {
        return it->second.get();
    }
    return nullptr;
}

void WorldManager::updateChunks(Vec3 worldCenter) {
    Vec3 centerChunkPos = worldToChunkPosition(worldCenter);
    AABB activeBox = {Vec3(centerChunkPos - Vec3(updateDistance)), Vec3(centerChunkPos + Vec3(updateDistance))};
    
    std::vector<Vec3> chunksToRemove;
    for (auto& [chunkPos, chunk] : chunkCache) {
        if (!AABBpointIn(chunkPos, activeBox)) {
            chunksToRemove.push_back(chunkPos);
        }
    }
    for (auto chunkPos : chunksToRemove) {
        availableOffsets.push(getChunk(chunkPos)->bufferOffset);
        chunkCache.erase(chunkPos);
    }

    int idx = 0;
    for (int z = -updateDistance; z <= updateDistance; z++) {
        for (int y = -updateDistance; y <= updateDistance; y++) {
            for (int x = -updateDistance; x <= updateDistance; x++, idx++) {
                Vec3 chunkPos = centerChunkPos + Vec3(x, y, z);
                auto chunk = getChunk(chunkPos);
                if (!chunk) {
                    int offset = availableOffsets.front();
                    availableOffsets.pop();
                    chunk = addChunk(chunkPos, offset);
                    chunkGenerator.generateChunk(chunk);
                }
                activeChunks[idx] = chunk;
            }
        }  
    }
}

// Voxel operations
void WorldManager::addVoxel(const Vec3& worldPosition, const Voxel& voxel) {
    auto chunk = getChunk(worldToChunkPosition(worldPosition));
    if (!chunk) {
        return;
    }
    if (chunk->addVoxel(worldPosition - chunk->worldPosition, voxel)) {
        chunk->isDirty = true;
        chunk->isEmpty = false;
    }
}

void WorldManager::removeVoxel(const Vec3& worldPosition) {
    auto chunk = getChunk(worldToChunkPosition(worldPosition));
    if (!chunk) return;
    if (chunk->removeVoxel(worldPosition - chunk->worldPosition)) {
        chunk->isDirty = true;
    }
}

Voxel& WorldManager::getVoxel(const Vec3& worldPosition) {
    auto chunk = getChunk(worldToChunkPosition(worldPosition));
    if (!chunk) {
        Voxel air = 0;
        return air;
    }
    return chunk->getVoxel(worldPosition - chunk->worldPosition);
}

bool WorldManager::positionIsSolid(const Vec3& worldPosition) {
    Voxel& voxel = getVoxel(worldPosition);
    return voxel.isSolid();
}

bool WorldManager::positionIsTransparent(const Vec3& worldPosition) {
    Voxel& voxel = getVoxel(worldPosition);
    return voxel.isTransparent();
}

bool WorldManager::worldRayDetection(const Vec3& startPoint, const Vec3& endPoint, Vec3& voxelPos, Vec3& normal) {
    Vec3 rayDir = normalise(endPoint - startPoint);
    Vec3 rayPos = startPoint;
    float rayLength = length(endPoint - startPoint);
    
    float tEntry = 0.0f, tEntryNext = 0.0f, tExit = 0.0f;

    voxelPos = floor(startPoint);
    normal = Vec3(0, 0, 0);
    while (tEntry <= rayLength) {
        auto voxel = getVoxel(voxelPos);
        if (!voxel.isTransparent()) {
            return true;
        }

        Vec3 step = sign(rayDir);
        step.x = (rayDir.x == 0) ? 1 : step.x;
        step.y = (rayDir.y == 0) ? 1 : step.y;
        step.z = (rayDir.z == 0) ? 1 : step.z;

        Vec3 voxelPosX = voxelPos + Vec3(step.x, 0, 0);
        Vec3 voxelPosY = voxelPos + Vec3(0, step.y, 0);
        Vec3 voxelPosZ = voxelPos + Vec3(0, 0, step.z);

        AABB voxelBoxX = { voxelPosX, voxelPosX + Vec3(1) };
        AABB voxelBoxY = { voxelPosY, voxelPosY + Vec3(1) };
        AABB voxelBoxZ = { voxelPosZ, voxelPosZ + Vec3(1) };

        if (AABBrayDetection(rayPos, rayDir, voxelBoxX, normal, tEntryNext, tExit)) {
            voxelPos = voxelPosX;
        } else if (AABBrayDetection(rayPos, rayDir, voxelBoxY, normal, tEntryNext, tExit)) {
            voxelPos = voxelPosY;
        } else if (AABBrayDetection(rayPos, rayDir, voxelBoxZ, normal, tEntryNext, tExit)) {
            voxelPos = voxelPosZ;
        } else {
            return false;
        }
        
        rayPos += rayDir * tEntryNext;
        tEntry += tEntryNext;
    }

    return false;
}
