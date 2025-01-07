#include "world/Chunk.h"
#include <queue>

int SubChunk::positionToIndex(const Vec3& localPosition) const {
    int idx = localPosition.x + (localPosition.y * size) + (localPosition.z * size * size);
    if (idx < 0 || idx >= (size * size * size)) {
        std::cerr << "subchunk does not contain the given local position: "; localPosition.print();
        return -1;
    } 
    
    return idx;
}

void SubChunk::forEachVoxel(std::function<void(const Vec3&, const Voxel&)> callback) const {
    int idx = 0;
    for (int z = 0; z < size; z++) {
        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++, idx++) {
                const Vec3 position(x, y, z);
                const Voxel& voxel = voxels[idx];
                callback(position, voxel);
            }
        }
    }
}

void SubChunk::forEachVoxel(std::function<void(const Vec3&, Voxel&)> callback) {
    int idx = 0;
    for (int z = 0; z < size; z++) {
        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++, idx++) {
                const Vec3 position(x, y, z);
                Voxel& voxel = voxels[idx];
                callback(position, voxel);
            }
        }
    }
}

// voxel manipulation with bounds and dirty flag handling
bool SubChunk::addVoxel(const Vec3& localPosition, const Voxel& newVoxel) {
    Voxel* voxel = getVoxelAt(localPosition);
    if (voxel->materialID != IDX_AIR) {
        std::cerr << "Attempted to replace a solid voxel" << std::endl;
        return false;
    }    
    *voxel = newVoxel;
    return true;
}

bool SubChunk::removeVoxel(const Vec3& localPosition) {
    Voxel* voxel = getVoxelAt(localPosition);
    if (voxel->materialID == IDX_AIR) {
        std::cerr << "Attempted to remove a non-solid voxel" << std::endl;
        return false;
    }
    *voxel = {0, 0, 0};
    return true;
}

Voxel* SubChunk::getVoxelAt(const Vec3& localPosition) {
    return &voxels[positionToIndex(localPosition)];
}

// bounds checking
bool SubChunk::positionInBounds(const Vec3& localPosition) const {
    return localPosition.x >= 0 && localPosition.x < size &&
           localPosition.y >= 0 && localPosition.y < size &&
           localPosition.z >= 0 && localPosition.z < size;
}

bool SubChunk::positionIsEdge(const Vec3& localPosition) const {
    if (!positionInBounds(localPosition)) return false;
    return localPosition.x == 0 || localPosition.x == size-1 ||
           localPosition.y == 0 || localPosition.y == size-1 ||
           localPosition.z == 0 || localPosition.z == size-1;
}