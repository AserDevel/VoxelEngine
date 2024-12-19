#pragma once

#include "utilities/standard.h"
#include "Voxels.h"
#include "Materials.h"
#include "rendering/Mesh.h"
#include "physics/AABB.h"

enum class ChunkState {
    PENDING,
    READY,
    MESHED,
    LOADED,
};

// cubic chunk
class Chunk {
public:
    Vec3 worldPosition;                          
    std::unordered_map<int, Voxel> activeVoxels; 
    AABB box;
    Mesh mesh = Mesh();
    bool isDirty = true;
    ChunkState state = ChunkState::PENDING;

    Chunk(Vec3 position, int chunkSize) : worldPosition(position), size(chunkSize) {
        box.min = worldPosition;
        box.max = worldPosition + (Vec3(1, 1, 1) * chunkSize);
    }

    ~Chunk() {}
    
    void addVoxel(const Vec3& worldPosition, const Voxel& voxel);

    void removeVoxel(const Vec3& worldPosition);
    
    Voxel* getVoxelAt(const Vec3& worldPosition);

    bool voxelExistsAt(const Vec3 worldPosition);
    
    Vec3 indexToPosition(int index);

    int positionToIndex(Vec3 localPosition);

    bool positionIsTransparent(Vec3 localPosition);

    bool positionInBounds(const Vec3& localPosition) const;

private:
    int size;    
};

