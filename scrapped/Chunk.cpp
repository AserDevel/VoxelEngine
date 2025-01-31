#include <queue>
#include "world/Chunk.h"

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
    if (worldHeight < maxDepth || worldHeight > maxHeight) return nullptr;
    int chunkHeight = worldToChunkHeight(worldHeight);
    if (subChunks.find(chunkHeight) == subChunks.end()) {
        Vec3 worldPosition = Vec3(this->worldPosition.x, chunkHeight * chunkSize, this->worldPosition.z);
        subChunks[chunkHeight] = std::make_unique<SubChunk>(worldPosition);
    }

    return subChunks[chunkHeight].get();
}

SubChunk* Chunk::getSubChunk(int worldHeight) const {
    if (worldHeight < maxDepth || worldHeight > maxHeight) return nullptr;
    int chunkHeight = worldToChunkHeight(worldHeight);
    auto it = subChunks.find(chunkHeight);
    if (it != subChunks.end()) {
        return it->second.get();
    }
    return nullptr;
}

// Check if a worldposition is valid within the chunk
bool Chunk::positionInBounds(const Vec3& worldPosition) const {
    Vec3 localPosition = worldPosition - this->worldPosition;
    return localPosition.x >= 0 && localPosition.x < chunkSize &&
           localPosition.z >= 0 && localPosition.z < chunkSize &&
           localPosition.y >= maxDepth && localPosition.y <= maxHeight;
}

bool Chunk::postitionIsEdge(const Vec3& worldPosition) const {
    if (!positionInBounds(worldPosition)) return false;
    Vec3 localPosition = worldPosition - this->worldPosition;
    return localPosition.x == 0 || localPosition.x == chunkSize - 1 ||
           localPosition.z == 0 || localPosition.z == chunkSize - 1;
}

void Chunk::markDirty(const Vec3& worldPosition) {
    auto subChunk = getSubChunk(worldPosition.y);
    if (!subChunk) return;
    isDirty = true;
    subChunk->isDirty = true;
    
    Vec3 localPosition = worldPosition - subChunk->worldPosition;
    if (worldPosition.y > maxDepth && localPosition.y == 0) {
        auto neighbour = getSubChunk(worldPosition.y - 1);
        if (!neighbour) return;
        neighbour->isDirty = true;
    } else if (worldPosition.y < maxHeight && localPosition.y == chunkSize - 1) {
        auto neighbour = getSubChunk(worldPosition.y + 1);
        if (!neighbour) return;
        neighbour->isDirty = true;
    }
}

// voxel manipulation
bool Chunk::addVoxel(const Vec3& worldPosition, const Voxel& voxel) {
    auto subChunk = addSubChunk(worldPosition.y);
    if (!subChunk) return false;
    return subChunk->addVoxel(worldPosition - subChunk->worldPosition, voxel);
}

bool Chunk::removeVoxel(const Vec3& worldPosition) {
    auto subChunk = getSubChunk(worldPosition.y);
    if (!subChunk) return false;
    return subChunk->removeVoxel(worldPosition - subChunk->worldPosition);
}

Voxel* Chunk::getVoxelAt(const Vec3& worldPosition) {
    auto subChunk = getSubChunk(worldPosition.y);
    if (!subChunk) return nullptr;
    return subChunk->getVoxelAt(worldPosition - subChunk->worldPosition);
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

void Chunk::setHeightAt(const Vec2& worldPosition2D, int newHeight) {    
    if (newHeight < maxDepth || newHeight > maxHeight) return;

    Vec2 localPosition2D = worldPosition2D - this->worldPosition.xz();
    int idx = localPosition2D.x + localPosition2D.z * chunkSize;
    if (idx < 0 || idx >= (chunkSize * chunkSize)) {
        std::cerr << "Error setting height at position: "; worldPosition2D.print();
        return;
    }
    heightMap[idx] = newHeight;
}

void Chunk::updateHeightAt(const Vec2& worldPosition2D) {
    for (int y = maxHeight; y >= maxDepth; y--) {
        Voxel* voxel = getVoxelAt(Vec3(worldPosition2D.x, y, worldPosition2D.z));
        if (!voxel || materials[*voxel].isTransparent) continue;
        setHeightAt(worldPosition2D, y);
        return;
    }
}

void Chunk::loadMeshes() {
    for (auto& [height, subChunk] : subChunks) {
        if (!subChunk->mesh.vertices.empty()) {
            subChunk->mesh.loadToGPU();
        }

        if (!subChunk->transparentMesh.vertices.empty()) {
            subChunk->transparentMesh.loadToGPU();
        }
        
        if (!subChunk->outlineMesh.vertices.empty()) {
            subChunk->outlineMesh.loadToGPU();
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

void Chunk::drawOutlines() {
    for (auto& [height, subChunk] : subChunks) {
        if (!subChunk->outlineMesh.vertices.empty()) {
            subChunk->outlineMesh.bind();
            subChunk->outlineMesh.draw();
        }
    }
}