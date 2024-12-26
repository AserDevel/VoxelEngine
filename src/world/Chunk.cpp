#include "world/Chunk.h"

void Chunk::addVoxel(const Vec3& localPosition, const Voxel& voxel) {
    if (positionInBounds(localPosition)) {
        voxels[positionToIndex(localPosition)] = voxel;
        int x = localPosition.x, z = localPosition.z;
        if (localPosition.y >= heightMap[x][z]) heightMap[x][z] = localPosition.y;
        isDirty = true;
    }
}

void Chunk::removeVoxel(const Vec3& localPosition) {
    int idx = positionToIndex(localPosition);
    if (voxels[idx].materialID != 0) {
        voxels[idx].materialID = 0;
        int x = localPosition.x, z = localPosition.z;
        if (localPosition.y == heightMap[x][z] - 1) heightMap[x][z] = localPosition.y - 1;
        isDirty = true;
    }
}

Voxel* Chunk::getVoxelAt(const Vec3& localPosition) {
    int idx = positionToIndex(localPosition);
    if (idx >= 0 && idx < (size * size * size)) {
        return &voxels[idx];
    } else {
        return nullptr; // or handle error as needed
    }
}

bool Chunk::positionIsSolid(const Vec3& localPosition) const {
    return voxels[positionToIndex(localPosition)].materialID != IDX_AIR;
}

Vec3 Chunk::indexToPosition(int index) const {
    Vec3 position;
    position.x = index % size;
    position.y = (index / size) % size;
    position.z = index / (size * size);

    return position;
}

int Chunk::positionToIndex(const Vec3& localPosition) const {
    return localPosition.x + (localPosition.y * size) + (localPosition.z * size * size);
}

bool Chunk::positionIsTransparent(const Vec3& localPosition) const {
    int idx = positionToIndex(localPosition);
    return (materials[voxels[idx].materialID].isTransparent);
}

// Helper function to check if a position is valid within the chunk
bool Chunk::positionInBounds(const Vec3& localPosition) const {
    return localPosition.x >= 0 && localPosition.x < size &&
           localPosition.y >= 0 && localPosition.y < size &&
           localPosition.z >= 0 && localPosition.z < size;
}

void Chunk::forEachVoxel(std::function<void(const Vec3&, const Voxel&)> callback) const {
    for (int x = 0; x < size; x++) {
        for (int y = 0; y < size; y++) {
            for (int z = 0; z < size; z++) {
                const Vec3& position = Vec3(x, y, z);
                const Voxel& voxel = voxels[positionToIndex(position)];
                if (voxel.materialID == IDX_AIR) continue;
                callback(position, voxel);
            }
        }
    }
}

void Chunk::forEachVoxel(std::function<void(const Vec3&, Voxel&)> callback) {
    for (int x = 0; x < size; x++) {
        for (int y = 0; y < size; y++) {
            for (int z = 0; z < size; z++) {
                const Vec3& position = Vec3(x, y, z);
                Voxel& voxel = voxels[positionToIndex(position)];
                if (voxel.materialID == IDX_AIR) continue;
                callback(position, voxel);
            }
        }
    }
}
