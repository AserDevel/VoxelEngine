#include "world/WorldManager.h"
#include <queue>

Vec3 WorldManager::worldToChunkPosition(const Vec3& worldPosition) const {
    int x = static_cast<int>(std::floor(worldPosition.x / chunkSize));
    int y = static_cast<int>(std::floor(worldPosition.y / chunkSize));
    int z = static_cast<int>(std::floor(worldPosition.z / chunkSize));
    return Vec3(x, y, z);
}

void WorldManager::removeChunk(const Vec3& chunkPosition) {
    chunkCache.erase(chunkPosition);
}

// Chunk management
std::shared_ptr<Chunk> WorldManager::addChunk(const Vec3& chunkPosition) {
    std::shared_ptr<Chunk> chunk = std::make_shared<Chunk>(chunkPosition * chunkSize);
    chunkCache[chunkPosition] = chunk;
    return chunk;
}

std::shared_ptr<Chunk> WorldManager::getChunkAt(const Vec3& chunkPosition) {
    auto it = chunkCache.find(chunkPosition);
    if (it != chunkCache.end()) {
        return it->second; // Return the found chunk
    }
    return nullptr;
}

bool WorldManager::chunkInCache(const Vec3& chunkPosition) {
    return chunkCache.find(chunkPosition) != chunkCache.end();
}

// Voxel operations
bool WorldManager::addVoxel(const Vec3& worldPosition, const Voxel& voxel) {
    auto chunk = getChunkAt(worldToChunkPosition(worldPosition));
    if (chunk == nullptr) return false;                                                  // Chunk is not in cache
    if (chunk->positionIsSolid(worldPosition - chunk->worldPosition)) return false;      // the position is occupied
    chunk->addVoxel(worldPosition - chunk->worldPosition, voxel);
    return true;
}

bool WorldManager::removeVoxel(const Vec3& worldPosition) {
    auto chunk = getChunkAt(worldToChunkPosition(worldPosition));
    if (chunk == nullptr) return false;                                              // Chunk is not in cache
    if (!chunk->positionIsSolid(worldPosition - chunk->worldPosition)) return false; // there is no voxel to be removed
    chunk->removeVoxel(worldPosition - chunk->worldPosition);
    
    // Update isDirty flag for potential neighbor chunks
    Vec3 localVoxelPos = worldPosition - chunk->worldPosition;
    if (localVoxelPos.x == 0 || localVoxelPos.x == chunkSize - 1 ||
        localVoxelPos.y == 0 || localVoxelPos.y == chunkSize - 1 ||
        localVoxelPos.z == 0 || localVoxelPos.z == chunkSize - 1) {
        for (int i = 0; i < 6; i++) {
            Vec3 neighborChunkPos = worldToChunkPosition(worldPosition + cubeNormals[i]);
            if (neighborChunkPos != worldToChunkPosition(chunk->worldPosition)) {
                if (chunkInCache(neighborChunkPos)) {
                    getChunkAt(neighborChunkPos)->isDirty = true;
                }
            }
        }
    }
    return true;
}

Voxel* WorldManager::getVoxelAt(const Vec3& worldPosition) {
    auto chunk = getChunkAt(worldToChunkPosition(worldPosition));
    if (chunk == nullptr) return nullptr;
    return chunk->getVoxelAt(worldPosition - chunk->worldPosition);
}

bool WorldManager::positionIsSolid(const Vec3& worldPosition) {
    auto chunk = getChunkAt(worldToChunkPosition(worldPosition));
    if (chunk == nullptr) return false;
    return chunk->positionIsSolid(worldPosition - chunk->worldPosition);
}

bool WorldManager::positionIsTransparent(const Vec3& worldPosition) {
    auto chunk = getChunkAt(worldToChunkPosition(worldPosition));
    if (chunk == nullptr) return true;
    return chunk->positionIsTransparent(worldPosition - chunk->worldPosition);
}

bool WorldManager::worldRayDetection(const Vec3& startPoint, const Vec3& endPoint, Vec3& voxelPos, Vec3& normal) {
    Vec3 rayDir = normalise(endPoint - startPoint);
    Vec3 rayPos = startPoint;
    float rayLength = length(endPoint - startPoint);
    
    float tEntry = 0.0f, tEntryNext = 0.0f, tExit = 0.0f;

    Vec3 boundary = Vec3(0.5, 0.5, 0.5);

    voxelPos = floor(startPoint + boundary);
    normal = Vec3(0, 0, 0);
    while (tEntry <= rayLength) {

        if (positionIsSolid(voxelPos)) {
            return true;
        }

        Vec3 step = sign(rayDir);
        step.x = (rayDir.x == 0) ? 1 : step.x;
        step.y = (rayDir.y == 0) ? 1 : step.y;
        step.z = (rayDir.z == 0) ? 1 : step.z;

        Vec3 voxelPosX = voxelPos + Vec3(step.x, 0, 0);
        Vec3 voxelPosY = voxelPos + Vec3(0, step.y, 0);
        Vec3 voxelPosZ = voxelPos + Vec3(0, 0, step.z);

        AABB voxelBoxX = { voxelPosX - boundary, voxelPosX + boundary };
        AABB voxelBoxY = { voxelPosY - boundary, voxelPosY + boundary };
        AABB voxelBoxZ = { voxelPosZ - boundary, voxelPosZ + boundary };

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
