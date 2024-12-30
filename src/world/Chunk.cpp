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

SubChunk* Chunk::addSubChunkAt(int worldHeight) {
    int chunkHeight = worldToChunkHeight(worldHeight);
    if (subChunks.find(chunkHeight) == subChunks.end()) {
        Vec3 worldPosition = Vec3(this->worldPosition.x, chunkHeight * chunkSize, this->worldPosition.z);
        subChunks[chunkHeight] = std::make_unique<SubChunk>(this, worldPosition);
    }

    return subChunks[chunkHeight].get();
}

SubChunk* Chunk::getSubChunkAt(int worldHeight) const {
    int chunkHeight = worldToChunkHeight(worldHeight);
    auto it = subChunks.find(chunkHeight);
    if (it != subChunks.end()) {
        return it->second.get();
    }
    return nullptr;
}

void Chunk::markDirty(const Vec3& worldPosition) {
    isDirty = true;
    auto subChunk = getSubChunkAt(worldPosition.y);
    if (subChunk) subChunk->isDirty = true;   
}

// Check if a worldposition is valid within the chunk
bool Chunk::positionInBounds(const Vec3& worldPosition) const {
    const Vec3 localPosition = worldPosition - this->worldPosition;
    return localPosition.x >= 0 && localPosition.x < chunkSize &&
           localPosition.z >= 0 && localPosition.z < chunkSize &&
           localPosition.y >= maxDepth && localPosition.y < maxHeight;
}

// voxel manipulation
bool Chunk::addVoxel(const Vec3& worldPosition, const Voxel& voxel) {
    auto subChunk = addSubChunkAt(worldPosition.y);
    if (subChunk == nullptr) {
        std::cerr << "Error adding voxel at the chunk level" << std::endl;
        return false;
    }
    Vec3 localPosition = worldPosition - subChunk->worldPosition;
    if (subChunk->addVoxel(localPosition, voxel)) {
        markDirty(worldPosition);
        if (localPosition.y == chunkSize - 1) {
            markDirty(worldPosition + Vec3(0, 1, 0));
        } else if (localPosition.y == 0 && worldPosition.y > maxDepth) {
            markDirty(worldPosition - Vec3(0, 1, 0));
        }
        return true;
    }
    return false;
}

bool Chunk::removeVoxel(const Vec3& worldPosition) {
    auto subChunk = getSubChunkAt(worldPosition.y);
    if (subChunk == nullptr) {
        std::cerr << "Error removing voxel at the chunk level" << std::endl;
        return false;
    }
    Vec3 localPosition = worldPosition - subChunk->worldPosition;
    if (subChunk->removeVoxel(localPosition)) {
        markDirty(worldPosition);
        if (localPosition.y == chunkSize - 1 && positionIsSolid(worldPosition + Vec3(0, 1, 0))) {
            markDirty(worldPosition + Vec3(0, 1, 0));
        } else if (localPosition.y == 0 && positionIsSolid(worldPosition - Vec3(0, 1, 0))) {
            markDirty(worldPosition - Vec3(0, 1, 0));
        }
        return true;
    }
    return false;
}

// position checking
bool Chunk::positionIsSolid(const Vec3& worldPosition) const {
    if (positionInBounds(worldPosition)) {
        auto subChunk = getSubChunkAt(worldPosition.y);
        if (subChunk == nullptr) return false;
        return subChunk->positionIsSolid(worldPosition - subChunk->worldPosition);
    } else {
        return worldManager->positionIsSolid(worldPosition);
    }
}

bool Chunk::positionIsTransparent(const Vec3& worldPosition) const {
    if (positionInBounds(worldPosition)) {
        auto subChunk = getSubChunkAt(worldPosition.y);
        if (subChunk == nullptr) return true;
        return subChunk->positionIsTransparent(worldPosition - subChunk->worldPosition);
    } else {
        return worldManager->positionIsTransparent(worldPosition);
    }
}

int Chunk::getHeightAt(const Vec2& worldPosition2D) const {
    Vec2 localPosition2D = worldPosition2D - Vec2(this->worldPosition.x, this->worldPosition.z);
    
    int idx = localPosition2D.x + localPosition2D.z * chunkSize;
    if (idx < 0 || idx >= (chunkSize * chunkSize)) {
        std::cerr << "Error getting height at position: "; worldPosition2D.print();
        return -1;
    }

    return heightMap[idx];    
}

void Chunk::setHeightAt(const Vec2& worldPosition2D, int height) {
    Vec2 localPosition2D = worldPosition2D - Vec2(this->worldPosition.x, this->worldPosition.z);
    
    int idx = localPosition2D.x + localPosition2D.z * chunkSize;
    if (idx < 0 || idx >= (chunkSize * chunkSize)) {
        std::cerr << "Error setting height at position: "; worldPosition2D.print();
        return;
    }

    heightMap[idx] = height;  
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

/*
void Chunk::propagateLight(const Vec3& sourceWorldPosition, uint8_t initialLightLevel, std::unordered_set<Vec3, Vec3Hash>& visited) {
    if (visited.count(sourceWorldPosition)) return; // Already processed
    visited.insert(sourceWorldPosition);

    auto voxel = getVoxelAt(sourceWorldPosition);
    if (!voxel) return;

    if (voxel->lightLevel >= initialLightLevel) return;
    voxel->lightLevel = initialLightLevel;

    uint8_t skyLight = (initialLightLevel & 0x0F);
    uint8_t blockLight = ((initialLightLevel >> 4) & 0x0F);

    if (skyLight > 0) skyLight--;
    if (blockLight > 0) blockLight--;

    uint8_t nextLightLevel = skyLight + (blockLight << 4);
    if (nextLightLevel == 0) return;

    for (int i = 0; i < 6; i++) {
        Vec3 newPos = sourceWorldPosition + cubeNormals[i];

        if (positionInBounds(newPos)) {
            if (!positionIsSolid(newPos)) {
                propagateLight(newPos, nextLightLevel, visited);
            }
        } else if (neighbourChunks[i]) {
            if (!visited.count(newPos) && !neighbourChunks[i]->positionIsSolid(newPos)) {
                neighbourChunks[i]->propagateLight(newPos, nextLightLevel, visited);
            }
        }
    }
}
*/

