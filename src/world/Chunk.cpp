#include <queue>
#include "world/WorldManager.h"

int Chunk::worldToChunkHeight(int worldHeight) const {
    int adjustedHeight = worldHeight + maxDepth;
    if (adjustedHeight < 0) {
        std::cerr << "Warning: Requested height exceeds max depth\n";
    } else if (adjustedHeight > (maxDepth + maxHeight)) {
        std::cerr << "Warning: Requested height exceeds max height\n";
    }
    int chunkHeight = static_cast<int>(std::floor(adjustedHeight / chunkSize));
    return chunkHeight;
}

SubChunk* Chunk::addSubChunk(int worldHeight) {
    int chunkHeight = worldToChunkHeight(worldHeight);
    if (subChunks.find(chunkHeight) == subChunks.end()) {
        Vec3 worldPosition = Vec3(this->worldPosition.x, chunkHeight * chunkSize, this->worldPosition.z);
        subChunks[chunkHeight] = std::make_unique<SubChunk>(this, worldPosition);
    }

    return subChunks[chunkHeight].get();
}

SubChunk* Chunk::getSubChunk(int worldHeight) const {
    int chunkHeight = worldToChunkHeight(worldHeight);
    auto it = subChunks.find(chunkHeight);
    if (it != subChunks.end()) {
        return it->second.get();
    }
    return nullptr;
}

SubChunk* Chunk::getNeighbourAt(const Vec3& worldPosition) const {
    if (positionInBounds(worldPosition)) {
        return getSubChunk(worldPosition.y);
    } else {
        auto neighbour = worldManager->getChunkAt(worldPosition);
        if (!neighbour) return nullptr;
        return neighbour->getNeighbourAt(worldPosition);
    }
}

void Chunk::markDirty(const Vec3& worldPosition) {
    isDirty = true;
    auto subChunk = getSubChunk(worldPosition.y);
    if (subChunk) subChunk->isDirty = true;
}

void Chunk::markDirtyNeighbours(const Vec3& worldPosition) {
    for (int i = 0; i < 6; i++) {
        Vec3 neighbourPos = worldPosition + cubeNormals[i];
        if (!positionInBounds(neighbourPos)) {
            auto neighbour = worldManager->getChunkAt(neighbourPos);
            if (neighbour) neighbour->markDirty(neighbourPos);
        } else {
            markDirty(neighbourPos);
        }
    } 
}

// Check if a worldposition is valid within the chunk
bool Chunk::positionInBounds(const Vec3& worldPosition) const {
    const Vec3 localPosition = worldPosition - this->worldPosition;
    return localPosition.x >= 0 && localPosition.x < chunkSize &&
           localPosition.z >= 0 && localPosition.z < chunkSize &&
           localPosition.y >= maxDepth && localPosition.y < maxHeight;
}

// voxel manipulation
void Chunk::addVoxel(const Vec3& worldPosition, const Voxel& voxel) {
    auto subChunk = addSubChunk(worldPosition.y);
    if (subChunk == nullptr) {
        std::cerr << "Error adding voxel at the chunk level" << std::endl;
        return;
    }
    Vec3 localPosition = worldPosition - subChunk->worldPosition;
    if (subChunk->addVoxel(localPosition, voxel)) {
        markDirty(worldPosition);
        if (subChunk->positionIsEdge(localPosition)) {
            markDirtyNeighbours(worldPosition);
        }
        Vec2 localPosition2D = Vec2(localPosition.x, localPosition.z);
        if (worldPosition.y > getHeightAt(localPosition2D)) {
            setHeightAt(localPosition2D, worldPosition.y);
        }
    }
}

void Chunk::removeVoxel(const Vec3& worldPosition) {
    auto subChunk = getSubChunk(worldPosition.y);
    if (subChunk == nullptr) {
        std::cerr << "Error removing voxel at the chunk level" << std::endl;
        return;
    }
    Vec3 localPosition = worldPosition - subChunk->worldPosition;
    if (subChunk->removeVoxel(localPosition)) {
        markDirty(worldPosition);
        if (subChunk->positionIsEdge(localPosition)) {
            markDirtyNeighbours(worldPosition);
        }
        Vec2 localPosition2D = Vec2(localPosition.x, localPosition.z);
        if (worldPosition.y == getHeightAt(localPosition2D)) {
            updateHeightAt(localPosition2D);
        } 
    }
}

Voxel* Chunk::getVoxelAt(const Vec3& worldPosition) {
    auto subChunk = getSubChunk(worldPosition.y);
    if (!subChunk) return nullptr;
    return subChunk->getVoxelAt(worldPosition - subChunk->worldPosition);
}

int Chunk::getHeightAt(const Vec2& localPosition2D) const {    
    int idx = localPosition2D.x + localPosition2D.z * chunkSize;
    if (idx < 0 || idx >= (chunkSize * chunkSize)) {
        std::cerr << "Error getting height at position: "; localPosition2D.print();
        return -1;
    }
    return heightMap[idx];    
}

void Chunk::setHeightAt(const Vec2& localPosition2D, int height) {    
    int idx = localPosition2D.x + localPosition2D.z * chunkSize;
    if (idx < 0 || idx >= (chunkSize * chunkSize)) {
        std::cerr << "Error setting height at position: "; localPosition2D.print();
        return;
    }
    heightMap[idx] = height;  
}

void Chunk::updateHeightAt(const Vec2& localPosition2D) {
    int x = this->worldPosition.x + localPosition2D.x;
    int z = this->worldPosition.z + localPosition2D.z;
    int y = maxHeight;
    while (y > maxDepth) {
        y--;
        Voxel* voxel = getVoxelAt(Vec3(x, y, z));
        if (!voxel) continue;
        if (voxel->materialID != IDX_AIR) break;
    }
    setHeightAt(localPosition2D, y);
}

void Chunk::generateMeshes() {
    for (auto& [height, subChunk] : subChunks) {
        if (subChunk->isDirty) {
            subChunk->generateMesh();
        }
    }
    state = ChunkState::MESHED;
    isDirty = false;
}

void Chunk::loadMeshes() {
    for (auto& [height, subChunk] : subChunks) {
        if (!subChunk->mesh.vertices.empty()) {
            subChunk->mesh.loadToGPU();
        }

        if (!subChunk->transparentMesh.vertices.empty()) {
            subChunk->transparentMesh.loadToGPU();
        }
    }
    state = ChunkState::LOADED;
}

void Chunk::draw() {
    for (auto& [height, subChunk] : subChunks) {
        if (!subChunk->mesh.vertices.empty()) {
            subChunk->mesh.bind();
            subChunk->mesh.draw();
        }
    }
}

void Chunk::drawTransparent() {
    for (auto& [height, subChunk] : subChunks) {
        if (!subChunk->transparentMesh.vertices.empty()) {
            subChunk->transparentMesh.bind();
            subChunk->transparentMesh.draw();
        }
    }
}

void Chunk::updateSkyLight() {
    for (auto& [height, subChunk] : subChunks) {
        subChunk->updateLightSources();
    }
    for (auto& [height, subChunk] : subChunks) {
        subChunk->updateLight();
    }
}

/*
void Chunk::propagateLight(const Vec3& worldPosition, uint8_t initialLightLevel, std::unordered_set<Vec3, Vec3Hash>& visited) {
    if (visited.count(worldPosition)) return; // Already processed
    visited.insert(worldPosition);

    auto voxel = getVoxelAt(worldPosition);
    if (!voxel) return;

    if (voxel->lightLevel > initialLightLevel) return;
    voxel->lightLevel = initialLightLevel;

    uint8_t skyLight = (initialLightLevel & 0x0F);
    uint8_t blockLight = ((initialLightLevel >> 4) & 0x0F);

    if (skyLight > 0) skyLight--;
    if (blockLight > 0) blockLight--;

    uint8_t nextLightLevel = skyLight + (blockLight << 4);
    if (nextLightLevel == 0) return;

    for (int i = 0; i < 6; i++) {
        Vec3 newPos = worldPosition + cubeNormals[i];

        if (positionInBounds(newPos)) {
            if (!positionIsSolid(newPos)) {
                propagateLight(newPos, nextLightLevel, visited);
            }
        } else {
            auto neighbourChunk = worldManager->getChunkAt(newPos);
            if (neighbourChunk && !neighbourChunk->positionIsSolid(newPos)) {
                neighbourChunk->propagateLight(newPos, nextLightLevel, visited);
            }
        }
    }
}
*/