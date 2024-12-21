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
    std::shared_ptr<Chunk> chunk = std::make_shared<Chunk>(chunkPosition * chunkSize, chunkSize);
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
    if (chunk == nullptr) return false;                         // Chunk is not in cache
    if (chunk->voxelExistsAt(worldPosition)) return false;      // the position is occupied
    chunk->addVoxel(worldPosition, voxel);
    return true;
}

bool WorldManager::removeVoxel(const Vec3& worldPosition) {
    auto chunk = getChunkAt(worldToChunkPosition(worldPosition));
    if (chunk == nullptr) return false;                     // Chunk is not in cache
    if (!chunk->voxelExistsAt(worldPosition)) return false; // there is no voxel to be removed
    chunk->removeVoxel(worldPosition);
    
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

Voxel* WorldManager::getVoxel(const Vec3& worldPosition) {
    auto chunk = getChunkAt(worldToChunkPosition(worldPosition));
    if (chunk == nullptr) return nullptr;
    return chunk->getVoxelAt(worldPosition);
}

bool WorldManager::voxelExistsAt(const Vec3& worldPosition) {
    auto chunk = getChunkAt(worldToChunkPosition(worldPosition));
    if (chunk == nullptr) return false;
    return chunk->voxelExistsAt(worldPosition);
}

bool WorldManager::getFirstVoxelCollision(const Vec3& startPoint, const Vec3& endPoint, Vec3& voxelPos) {
    if (voxelExistsAt(startPoint)) {
        voxelPos = startPoint;
        return true;
    }

    Vec3 rayDir = normalise(endPoint - startPoint);
    Vec3 rayPos = startPoint;
    float rayLength = length(endPoint - startPoint);
    
    Vec3 normal;
    float tEntry, tEntryNext, tExit;

    // get all chunks that collide with the ray sorted by entry times
    Vec3 chunkPos = worldToChunkPosition(startPoint);
    std::queue<std::shared_ptr<Chunk>> chunksToCheck;
    while (tEntry <= rayLength) {
        chunksToCheck.push(getChunkAt(chunkPos));

        Vec3 step = sign(rayDir);
        if (step.x == 0) step.x = 1; if (step.y == 0) step.y = 1; if (step.z == 0) step.z = 1;

        auto chunkX = getChunkAt(chunkPos + Vec3(step.x, 0, 0));
        auto chunkY = getChunkAt(chunkPos + Vec3(0, step.y, 0));
        auto chunkZ = getChunkAt(chunkPos + Vec3(0, 0, step.z));

        if (!chunkX || !chunkY || !chunkZ) break;

        if (AABBrayDetection(rayPos, rayDir, chunkX->box, normal, tEntryNext, tExit)) {
            chunkPos = worldToChunkPosition(chunkX->worldPosition);
        } else if (AABBrayDetection(rayPos, rayDir, chunkY->box, normal, tEntryNext, tExit)) {
            chunkPos = worldToChunkPosition(chunkY->worldPosition);
        } else if (AABBrayDetection(rayPos, rayDir, chunkZ->box, normal, tEntryNext, tExit)) {
            chunkPos = worldToChunkPosition(chunkZ->worldPosition);
        } else {
            break;
        }
        
        rayPos += rayDir * tEntryNext;
        tEntry += tEntryNext;
    }
    
    float firstEntry = INF_FLOAT;
    bool collision = false;

    // Check each chunk from front to back
    while (!chunksToCheck.empty()) {
        auto chunk = chunksToCheck.front();
        chunksToCheck.pop();

        if (chunk == nullptr) continue;

        Vec3 chunkWorldPos = chunk->worldPosition;
        for (auto [index, voxel] : chunk->activeVoxels) {
            Vec3 pos = chunkWorldPos + chunk->indexToPosition(index);
            
            AABB box = { pos, pos + Vec3(1, 1, 1) };
            if (AABBrayDetection(startPoint, rayDir, box, normal, tEntry, tExit)) {
                // Ignore negative tEntry (behind the ray origin) and those furter than raylength
                if (tEntry > 0 && tEntry <= rayLength && tEntry < firstEntry) {
                    firstEntry = tEntry;
                    voxelPos = pos;
                    collision = true;
                }
            }
        }
        if (collision) {
            return true;
        }
    }

    return false;
}
