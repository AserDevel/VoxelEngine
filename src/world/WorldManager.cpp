#include "world/WorldManager.h"
#include <queue>

Vec3 WorldManager::worldToChunkPosition(const Vec3& worldPosition) const {
    return floor(worldPosition / chunkSize);
}

Chunk* WorldManager::addChunk(const Vec3& chunkPosition, int bufferOffset) {
    Vec3 worldPosition = chunkPosition * chunkSize;
    activeChunks[chunkPosition] = std::make_unique<Chunk>(worldPosition, bufferOffset);
    return activeChunks[chunkPosition].get();
}

Chunk* WorldManager::getChunk(const Vec3& chunkPosition) const {
    auto it = activeChunks.find(chunkPosition);
    if (it != activeChunks.end()) {
        return it->second.get();
    }
    return nullptr;
}

ChunkColumn* WorldManager::addColumn(Vec2 columnPosition) {
    activeColumns[columnPosition] = std::make_unique<ChunkColumn>(columnPosition * chunkSize);
    return activeColumns[columnPosition].get();
}

ChunkColumn* WorldManager::getColumn(Vec2 columnPosition) const {
    auto it = activeColumns.find(columnPosition);
    if (it != activeColumns.end()) {
        return it->second.get();
    }
    return nullptr;
}

void WorldManager::updateChunks(Vec3 worldCenter) {
    Vec3 centerChunkPos = worldToChunkPosition(worldCenter);
    AABB activeBox = {Vec3(centerChunkPos - Vec3(updateDistance)), Vec3(centerChunkPos + Vec3(updateDistance))};
    AABB2D activeBox2D = {activeBox.min.xz(), activeBox.max.xz()};
    worldBasePos = activeBox.min * chunkSize;

    // cleanup out of range chunks
    std::vector<Vec3> chunksToRemove;
    for (auto& [chunkPos, chunk] : activeChunks) {
        if (!AABBpointIn(chunkPos, activeBox) && chunk->state != PENDING) {
            chunksToRemove.push_back(chunkPos);
        }
    }
    for (auto chunkPos : chunksToRemove) {
        availableOffsets.push(getChunk(chunkPos)->bufferOffset);
        auto column = getColumn(chunkPos.xz());
        column->dependencyCount--;
        activeChunks.erase(chunkPos);
    }

    // cleanup out of range columns
    std::vector<Vec2> columnsToRemove;
    for (auto& [columnPos, column] : activeColumns) {
        if (!AABBpointIn2D(columnPos, activeBox2D) && column->dependencyCount == 0) {
            columnsToRemove.push_back(columnPos);
        }
    }
    for (auto columnPos : columnsToRemove) {
        activeColumns.erase(columnPos);
    }

    // update and generate new chunks, and update the flat chunk array
    int idx = 0;
    for (int z = -updateDistance; z <= updateDistance; z++) {
        for (int y = -updateDistance; y <= updateDistance; y++) {
            for (int x = -updateDistance; x <= updateDistance; x++, idx++) {
                Vec3 chunkPos = centerChunkPos + Vec3(x, y, z);
                auto chunk = getChunk(chunkPos);
                if (!chunk) {
                    if (availableOffsets.empty()) continue;

                    int offset = availableOffsets.front();
                    availableOffsets.pop();
                    chunk = addChunk(chunkPos, offset);
                    
                    auto column = getColumn(chunkPos.xz());
                    if (!column) {
                        column = addColumn(chunkPos.xz());
                        chunkGenerator.generateChunkColumn(column);
                    }
                    column->dependencyCount++;
                    
                    chunk->state = PENDING;
                    threadManager.addTask([this, chunk]() {
                        chunkGenerator.generateChunk(chunk);
                    });
                } 
                chunks[idx] = chunk;
            }
        }  
    }

    // Generate features for chunks where all neighbours are initiated
    for (int i = 0; i < numChunks; i++) {
        auto chunk = chunks[i];
        if (chunk && chunk->state == GENERATED && neighboursReady(chunk)) {
            chunkGenerator.generateFeatures(chunk);
        }
    }
}

bool WorldManager::neighboursReady(Chunk* chunk) {
    Vec3 centerChunkPos = worldToChunkPosition(chunk->worldPosition);
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            for (int z = -1; z <= 1; z++) {
                if (x == 0 && y == 0 && z == 0) continue;
                Vec3 chunkPos = centerChunkPos + Vec3(x, y, z);
                Chunk* neighbour = getChunk(chunkPos);
                if (!neighbour || neighbour->state == PENDING) return false;
            }
        }
    }
    return true;
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

Voxel* WorldManager::getVoxel(const Vec3& worldPosition) {
    auto chunk = getChunk(worldToChunkPosition(worldPosition));
    if (!chunk) {
        return nullptr;
    }
    return chunk->getVoxel(worldPosition - chunk->worldPosition);
}

bool WorldManager::positionIsSolid(const Vec3& worldPosition) {
    Voxel* voxel = getVoxel(worldPosition);
    if (!voxel) return false;
    return voxel->isSolid();
}

bool WorldManager::positionIsTransparent(const Vec3& worldPosition) {
    Voxel* voxel = getVoxel(worldPosition);
    if (!voxel) return true;
    return voxel->isTransparent();
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
        if (voxel && voxel->isSolid()) {
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
