#include "world/WorldManager.h"
#include <queue>

Vec2 WorldManager::worldToChunkPosition(const Vec3& worldPosition) const {
    int x = static_cast<int>(std::floor(worldPosition.x / chunkSize));
    int z = static_cast<int>(std::floor(worldPosition.z / chunkSize));
    return Vec2(x, z);
}

Vec2 WorldManager::worldToChunkPosition(const Vec2& worldPosition2D) const {
    int x = static_cast<int>(std::floor(worldPosition2D.x / chunkSize));
    int z = static_cast<int>(std::floor(worldPosition2D.z / chunkSize));
    return Vec2(x, z);
}

void WorldManager::removeChunk(const Vec2& chunkPosition2D) {
    chunkCache.erase(chunkPosition2D);
}

// Chunk management
Chunk* WorldManager::addChunk(const Vec2& chunkPosition2D) {
    chunkCache[chunkPosition2D] = std::make_unique<Chunk>(
        Vec3(chunkPosition2D.x * chunkSize, MAX_DEPTH, chunkPosition2D.z * chunkSize));
    return chunkCache[chunkPosition2D].get();
}

Chunk* WorldManager::getChunk(const Vec2& chunkPosition2D) const {
    auto it = chunkCache.find(chunkPosition2D);
    if (it != chunkCache.end()) {
        return it->second.get();
    }
    return nullptr;
}

bool WorldManager::chunkInCache(const Vec2& chunkPosition2D) {
    return chunkCache.find(chunkPosition2D) != chunkCache.end();
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
                chunk = getChunk(chunkPos);
            }
            
            if (chunk->state == ChunkState::PENDING || !neighboursReady(chunk)) continue;
            
            // Mesh all chunks that are either dirty or newly generated and ready for meshing
            if (chunk->state == ChunkState::GENERATED) {
                chunk->state = ChunkState::PENDING;
                threadManager.addTask([this, chunk]() {
                    meshGenerator.generateChunkMeshes(chunk);
                });
                continue;
            }

            if (chunk->isDirty) {
                meshGenerator.generateChunkMeshes(chunk);
            }
            
            if (chunk->state == ChunkState::MESHED) {   
                chunk->loadMeshes();
            }
        }  
    }
}

void WorldManager::markDirty(const Vec3& worldPosition) {
    auto chunk = getChunk(worldToChunkPosition(worldPosition));
    if (!chunk) return;
    chunk->markDirty(worldPosition);

    if (chunk->postitionIsEdge(worldPosition)) {
        const Vec3 neighbours[4] = {Vec3(1, 0, 0), Vec3(-1, 0, 0), Vec3(0, 0, 1), Vec3(0, 0, -1)}; 
        for (int n = 0; n < 4; n++) {
            Vec3 neighbourPos = worldPosition + neighbours[n];
            if (!chunk->positionInBounds(neighbourPos)) {
                auto neighbour = getChunk(worldToChunkPosition(neighbourPos));
                if (!neighbour) continue;
                neighbour->markDirty(neighbourPos);
            }
        }
    }
}

bool WorldManager::neighboursReady(Chunk* chunk) {
    if (!chunk) return false;
    
    Vec2 neighbourPositions[8] = { Vec2(1, 0), Vec2(-1, 0), Vec2(0, 1), Vec2(0, -1), Vec2(1, 1), Vec2(1, -1), Vec2(-1, 1), Vec2(-1, -1) };    
    for (int i = 0; i < 8; i++) {
        auto neighbourChunk = getChunk(worldToChunkPosition(chunk->worldPosition) + neighbourPositions[i]);
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
    for (auto& [chunkPos, chunk] : chunkCache) {
        if (chunk->state == ChunkState::LOADED) {
            loadedChunks.push_back(chunk.get());
        }
    }
    return loadedChunks;
}

// Voxel operations
void WorldManager::addVoxel(const Vec3& worldPosition, const Voxel& voxel) {
    auto chunk = getChunk(worldToChunkPosition(worldPosition));
    if (chunk == nullptr) {
        std::cerr << "Error adding voxel at world level" << std::endl;
        return;
    }
    uint8_t initialLightLevel = getLightLevelAt(worldPosition);
    if (chunk->addVoxel(worldPosition, voxel)) {
        markDirty(worldPosition);
        Vec2 worldPosition2D = Vec2(worldPosition.x, worldPosition.z);
        if (chunk->getHeightAt(worldPosition2D) < worldPosition.y) {
            chunk->setHeightAt(worldPosition2D, worldPosition.y);
            updateSkyLightAt(worldPosition2D);
        } else {
            lightGenerator.removeLight(worldPosition, initialLightLevel);
        }
    }
}

void WorldManager::removeVoxel(const Vec3& worldPosition) {
    auto chunk = getChunk(worldToChunkPosition(worldPosition));
    if (chunk == nullptr) {
        std::cerr << "Error removing voxel at world level" << std::endl;
        return;
    }
    if (chunk->removeVoxel(worldPosition)) {
        markDirty(worldPosition);
        Vec2 worldPosition2D = Vec2(worldPosition.x, worldPosition.z);
        if (chunk->getHeightAt(worldPosition2D) == worldPosition.y) {
            chunk->updateHeightAt(worldPosition2D);
            updateSkyLightAt(worldPosition2D);
        } else {
            uint8_t initialLightLevel = lightGenerator.calculateLightAt(worldPosition);
            lightGenerator.propagateLight(worldPosition, initialLightLevel);
        }
    }
}

Voxel* WorldManager::getVoxelAt(const Vec3& worldPosition) {
    auto chunk = getChunk(worldToChunkPosition(worldPosition));
    if (chunk == nullptr) {
        std::cerr << "Error getting voxel at world level: "; worldPosition.print();
        return nullptr;
    }
    return chunk->getVoxelAt(worldPosition);
}

bool WorldManager::positionIsSolid(const Vec3& worldPosition) {
    Voxel* voxel = getVoxelAt(worldPosition);
    if (!voxel) return false;
    return voxel->materialID != IDX_AIR;
}

bool WorldManager::positionIsTransparent(const Vec3& worldPosition) {
    Voxel* voxel = getVoxelAt(worldPosition);
    if (!voxel) return true;
    return materials[voxel->materialID].isTransparent;
}

uint8_t WorldManager::getLightLevelAt(const Vec3& worldPosition) {
    Voxel* voxel = getVoxelAt(worldPosition);
    if (!voxel) return 0x0F;
    return voxel->lightLevel;
}

void WorldManager::updateSkyLightAt(const Vec2 worldPosition2D) {
    std::vector<Vec3> lightToRemove;
    std::vector<Vec3> lightToPropagate;
    auto chunk = getChunk(worldToChunkPosition(worldPosition2D));
    if (!chunk) return;
    int height = chunk->getHeightAt(worldPosition2D);
    for (int y = MAX_HEIGHT; y >= MAX_DEPTH; y--) {
        Vec3 wp = Vec3(worldPosition2D.x, y, worldPosition2D.z);
        Voxel* voxel = chunk->getVoxelAt(wp);
        if (!voxel) continue;
        if (voxel->materialID == IDX_AIR) {
            if (y > height) {
                if ((voxel->lightLevel & 0x0F) != 0x0F) {
                    lightToPropagate.push_back(wp);
                } 
            } else {
                if ((voxel->lightLevel & 0x0F) == 0x0F) {
                    voxel->lightLevel = 0xE;
                    lightToRemove.push_back(wp);
                } 
            }
        }
    }
    for (auto& worldPos : lightToPropagate) {
        lightGenerator.propagateLight(worldPos, 0x0F);
    }
    for (auto& worldPos : lightToRemove) {
        lightGenerator.removeLight(worldPos, 0x0F);
    }
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
        auto voxel = getVoxelAt(voxelPos);
        if (voxel && voxel->materialID != IDX_AIR) {
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
