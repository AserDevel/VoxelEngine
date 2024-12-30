#include "world/WorldManager.h"
#include <queue>

Vec2 WorldManager::worldToChunkPosition(const Vec3& worldPosition) const {
    int x = static_cast<int>(std::floor(worldPosition.x / chunkSize));
    int z = static_cast<int>(std::floor(worldPosition.z / chunkSize));
    return Vec2(x, z);
}

void WorldManager::updateChunks(Vec3 worldCenter) {
    Vec2 worldCenter2D = Vec2(worldCenter.x, worldCenter.z);
    Vec2 centerChunkPos = worldToChunkPosition(worldCenter);
    
    for (int x = -updateDistance; x <= updateDistance; x++) {
        for (int z = -updateDistance; z <= updateDistance; z++) {
            Vec2 chunkPos = centerChunkPos + Vec2(x, z);

            Vec2 chunkWorldCenter2D = chunkPos * chunkSize + Vec2(chunkSize / 2.0f, chunkSize / 2.0f);

            float distance = length(chunkWorldCenter2D - worldCenter2D);            
            if (distance > updateDistance * chunkSize) {
                if (chunkInCache(chunkPos)) {
                    removeChunk(chunkPos); // Remove chunk from cache
                } 
                continue;
            } 

            Chunk* chunk;
            if (!chunkInCache(chunkPos)) {
                chunk = addChunk(chunkPos);
                chunkGenerator.generateChunk(chunk);
            } else {
                chunk = getChunkAt(chunkPos);
            }

            if (chunk->state == ChunkState::PENDING || !neighboursReady(chunk)) continue;
            
            // Mesh all chunks that are either dirty or newly generated and ready for meshing
            if (chunk->state == ChunkState::GENERATED) {
                chunk->state = ChunkState::PENDING;
                threadManager.addTask([this, chunk]() {
                    chunk->generateMeshes();
                });
                continue;
            }
            
            if (chunk->isDirty) {    
                chunk->generateMeshes();
            }

            if (chunk->state == ChunkState::MESHED) {   
                chunk->loadMeshes();
            }
        }  
    }
}

bool WorldManager::neighboursReady(Chunk* chunk) {
    if (!chunk) return false;
    
    Vec2 neighbourDirs[8] = { Vec2(1, 0), Vec2(-1, 0), Vec2(0, 1), Vec2(0, -1), Vec2(1, 1), Vec2(1, -1), Vec2(-1, 1), Vec2(-1, -1) };    
    for (int i = 0; i < 8; i++) {
        auto neighbourChunk = getChunkAt(chunk->chunkPosition2D + neighbourDirs[i]);
        if (neighbourChunk == nullptr) {
            return false;
        } else {
            if (neighbourChunk->state == ChunkState::PENDING) return false;
        }
    }

    return true;
}

std::vector<Chunk*> WorldManager::getLoadedChunks() {
    std::vector<Chunk*> loadedChunks;
    for (auto& [pos, chunk] : chunkCache) {
        if (chunk->state == ChunkState::LOADED) {
            loadedChunks.push_back(chunk.get());
        }
    }
    return loadedChunks;
}

void WorldManager::removeChunk(const Vec2& chunkPosition2D) {
    chunkCache.erase(chunkPosition2D);
}

// Chunk management
Chunk* WorldManager::addChunk(const Vec2& chunkPosition2D) {
    chunkCache[chunkPosition2D] = std::make_unique<Chunk>(this, chunkPosition2D);
    return chunkCache[chunkPosition2D].get();
}

Chunk* WorldManager::getChunkAt(const Vec2& chunkPosition2D) const {
    auto it = chunkCache.find(chunkPosition2D);
    if (it != chunkCache.end()) {
        return it->second.get();
    }
    return nullptr;
}

bool WorldManager::chunkInCache(const Vec2& chunkPosition2D) {
    return chunkCache.find(chunkPosition2D) != chunkCache.end();
}

// Voxel operations
void WorldManager::addVoxel(const Vec3& worldPosition, const Voxel& voxel) {
    auto chunk = getChunkAt(worldToChunkPosition(worldPosition));
    if (chunk == nullptr) {
        std::cerr << "Error adding voxel at world level" << std::endl;
        return;
    }
    if (chunk->addVoxel(worldPosition, voxel)) {
        for (int i = 0; i < 4; i++) {
            auto neighbourVoxelPos = worldPosition + cubeNormals[i];
            auto neighbour = getChunkAt(worldToChunkPosition(neighbourVoxelPos));
            if (neighbour == nullptr) continue;
            if (chunk != neighbour && neighbour->positionIsSolid(neighbourVoxelPos)) {
                neighbour->markDirty(neighbourVoxelPos);
            }
        }
    }
}

void WorldManager::removeVoxel(const Vec3& worldPosition) {
    auto chunk = getChunkAt(worldToChunkPosition(worldPosition));
    if (chunk == nullptr) {
        std::cerr << "Error removing voxel at world level" << std::endl;
        return;
    }
    if (chunk->removeVoxel(worldPosition)) {
        for (int i = 0; i < 4; i++) {
            auto neighbourVoxelPos = worldPosition + cubeNormals[i];
            auto neighbour = getChunkAt(worldToChunkPosition(neighbourVoxelPos));
            if (neighbour == nullptr) continue;
            if (chunk != neighbour && neighbour->positionIsSolid(neighbourVoxelPos)) {
                neighbour->markDirty(neighbourVoxelPos);
            }
        }
    }
}

bool WorldManager::positionIsSolid(const Vec3& worldPosition) const {
    if (worldPosition.y < MAX_DEPTH || worldPosition.y > MAX_HEIGHT) return false;
    auto chunk = getChunkAt(worldToChunkPosition(worldPosition));
    if (chunk == nullptr) return false;
    return chunk->positionIsSolid(worldPosition);
}

bool WorldManager::positionIsTransparent(const Vec3& worldPosition) const {
    if (worldPosition.y < MAX_DEPTH || worldPosition.y > MAX_HEIGHT) return true;
    auto chunk = getChunkAt(worldToChunkPosition(worldPosition));
    if (chunk == nullptr) return true;
    return chunk->positionIsTransparent(worldPosition);
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
