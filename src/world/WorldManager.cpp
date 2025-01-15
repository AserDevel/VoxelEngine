#include "world/WorldManager.h"
#include <queue>

Vec2 WorldManager::worldToChunkPosition(const Vec3& worldPosition) const {
    return floor(worldPosition.xz() / chunkSize);
}

Vec2 WorldManager::worldToChunkPosition(const Vec2& worldPosition2D) const {
    return floor(worldPosition2D / chunkSize);
}

Chunk* WorldManager::addChunk(const Vec2& chunkPosition2D) {
    Vec3 worldPosition = Vec3(chunkPosition2D.x * chunkSize, MAX_DEPTH, chunkPosition2D.z * chunkSize); 
    chunkCache[chunkPosition2D] = std::make_unique<Chunk>(worldPosition);
    return chunkCache[chunkPosition2D].get();
}

Chunk* WorldManager::getChunk(const Vec2& chunkPosition2D) const {
    auto it = chunkCache.find(chunkPosition2D);
    if (it != chunkCache.end()) {
        return it->second.get();
    }
    return nullptr;
}

void WorldManager::updateChunks(Vec3 worldCenter) {
    Vec2 worldCenter2D = worldCenter.xz();
    Vec2 centerChunkPos = worldToChunkPosition(worldCenter);
    
    for (int x = -updateDistance; x <= updateDistance; x++) {
        for (int z = -updateDistance; z <= updateDistance; z++) {
            Vec2 chunkPos = centerChunkPos + Vec2(x, z);

            Vec2 chunkWorldCenter2D = chunkPos * chunkSize + Vec2(chunkSize / 2.0f, chunkSize / 2.0f);
            
            float distance = length(chunkWorldCenter2D - worldCenter2D); 
            if (distance > updateDistance * chunkSize) continue;

            auto chunk = getChunk(chunkPos);
            if (!chunk) {
                chunk = addChunk(chunkPos);
                chunkGenerator.generateChunk(chunk); // Generate general height and biome
            }

            if (chunk->state == ChunkState::PENDING || chunk->state == ChunkState::REMESHING) continue;

            // Generate features and mark chunk as it's ready for meshing
            if (chunk->state == ChunkState::GENERATED && neighboursGenerated(chunk)) {
                chunkGenerator.generateFeatures(chunk);
            }
                        
            // Mesh newly generated chunks
            if (chunk->state == ChunkState::READY && neighboursReady(chunk)) {
                chunk->state = ChunkState::PENDING;
                threadManager.addTask([this, chunk]() {
                    //int start, end;
                    //start = SDL_GetTicks();
                    meshGenerator.generateChunkMeshes(chunk);
                    //end = SDL_GetTicks();
                    //std::cout << "Chunk generated in: " << end - start << " ticks" << std::endl;
                });
            }
            
            // Load newly meshed chunks
            if (chunk->state == ChunkState::MESHED) {   
                chunk->loadMeshes();
            }

            // Remesh dirty chunks
            if (chunk->isDirty && chunk->state >= ChunkState::LOADED) {
                chunk->state = ChunkState::REMESHING;
                threadManager.addTask([this, chunk]() {
                    meshGenerator.generateChunkMeshes(chunk);
                });
            }
        }  
    }

    // Cleanup out of range chunks
    std::vector<Vec2> chunksToRemove;
    for (auto& [chunkPos, chunk] : chunkCache) {
        Vec2 chunkWorldCenter2D = chunkPos * chunkSize + Vec2(chunkSize / 2.0f, chunkSize / 2.0f);
        float distance = length(chunkWorldCenter2D - worldCenter2D);
        if (distance > updateDistance * chunkSize) {
            if (chunk->state != ChunkState::PENDING && !neighbourIsPending(chunk.get())) {
                chunksToRemove.push_back(chunkPos);
            }
        }
    }
    for (const auto& chunkPos : chunksToRemove) {
        chunkCache.erase(chunkPos);
    }
}

const Vec2 neighbourPositions[8] = { Vec2(1, 0), Vec2(-1, 0), Vec2(0, 1), Vec2(0, -1), Vec2(1, 1), Vec2(1, -1), Vec2(-1, 1), Vec2(-1, -1) };    

bool WorldManager::neighboursGenerated(Chunk* chunk) {    
    for (int i = 0; i < 8; i++) {
        auto neighbourChunk = getChunk(worldToChunkPosition(chunk->worldPosition) + neighbourPositions[i]);
        if (!neighbourChunk || neighbourChunk->state < ChunkState::GENERATED) return false;
    }
    
    return true;
}

bool WorldManager::neighboursReady(Chunk* chunk) {    
    for (int i = 0; i < 8; i++) {
        auto neighbourChunk = getChunk(worldToChunkPosition(chunk->worldPosition) + neighbourPositions[i]);
        if (!neighbourChunk || neighbourChunk->state < ChunkState::READY) return false;
    }
    
    return true;
}

bool WorldManager::neighbourIsPending(Chunk* chunk) {    
    for (int i = 0; i < 8; i++) {
        auto neighbourChunk = getChunk(worldToChunkPosition(chunk->worldPosition) + neighbourPositions[i]);
        if (!neighbourChunk) continue;
        if (neighbourChunk->state == ChunkState::PENDING) return true;
    }
    
    return false;
}

std::vector<Chunk*> WorldManager::getLoadedChunks() {
    std::vector<Chunk*> loadedChunks;
    for (auto& [chunkPos, chunk] : chunkCache) {
        if (chunk->state >= ChunkState::LOADED) {
            loadedChunks.push_back(chunk.get());
        }
    }
    return loadedChunks;
}

// Voxel operations
void WorldManager::addVoxel(const Vec3& worldPosition, const Voxel& voxel) {
    auto chunk = getChunk(worldToChunkPosition(worldPosition));
    if (!chunk) {
        return;
    }
    uint8_t initialLightLevel = getLightLevelAt(worldPosition);
    if (chunk->addVoxel(worldPosition, voxel)) {
        markDirty(worldPosition);
        Vec2 worldPosition2D = worldPosition.xz();
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
    if (!chunk) return;
    if (chunk->removeVoxel(worldPosition)) {
        markDirty(worldPosition);
        Vec2 worldPosition2D = worldPosition.xz();
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
    if (!chunk) {
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
        if (materials[voxel->materialID].isTransparent) {
            if (y > height) {
                if (voxel->materialID == IDX_AIR && (voxel->lightLevel & 0x0F) != 15) {
                    lightToPropagate.push_back(wp);
                }
            } else if ((voxel->lightLevel & 0x0F) == 15) {
                voxel->lightLevel--;
                lightToRemove.push_back(wp);
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
        if (voxel && !materials[voxel->materialID].isTransparent) {
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
