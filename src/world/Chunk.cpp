#include "world/Chunk.h"

void Chunk::addVoxel(const Vec3& worldPosition, const Voxel& voxel) {
    Vec3 localPosition = worldPosition - this->worldPosition;
    if (positionInBounds(localPosition)) {
        activeVoxels[positionToIndex(localPosition)] = voxel;
        isDirty = true;
    }
}

void Chunk::removeVoxel(const Vec3& worldPosition) {
    Vec3 localPosition = worldPosition - this->worldPosition;
    auto idx = positionToIndex(localPosition);
    if (activeVoxels.find(idx) != activeVoxels.end()) {
        activeVoxels.erase(idx);
        isDirty = true;
    }
}

Voxel* Chunk::getVoxelAt(const Vec3& worldPosition) {
    Vec3 localPosition = worldPosition - this->worldPosition;
    auto idx = positionToIndex(localPosition);
    auto it = activeVoxels.find(idx);
    if (it != activeVoxels.end()) {
        return &it->second;
    }
    return nullptr; // or handle error as needed
}

bool Chunk::voxelExistsAt(const Vec3 worldPosition) {
    Vec3 localPosition = worldPosition - this->worldPosition;
    auto idx = positionToIndex(localPosition);
    return activeVoxels.find(idx) != activeVoxels.end();
}

Vec3 Chunk::indexToPosition(int index) {
    Vec3 position;
    position.x = index % size;
    position.y = (index / size) % size;
    position.z = index / (size * size);

    if (!positionInBounds(position)) return Vec3(0,0,0);

    return position;
}

int Chunk::positionToIndex(Vec3 localPosition) {
    if (!positionInBounds(localPosition)) return 0;

    int x = std::floor(localPosition.x);
    int y = std::floor(localPosition.y);
    int z = std::floor(localPosition.z);

    return x + (y * size) + (z * size * size);
}

bool Chunk::positionIsTransparent(Vec3 localPosition) {
    return this->activeVoxels.find(this->positionToIndex(localPosition)) == this->activeVoxels.end();
}

// Helper function to check if a position is valid within the chunk
bool Chunk::positionInBounds(const Vec3& localPosition) const {
    return localPosition.x >= 0 && localPosition.x < size &&
           localPosition.y >= 0 && localPosition.y < size &&
           localPosition.z >= 0 && localPosition.z < size;
}