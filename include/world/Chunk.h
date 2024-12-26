#pragma once

#include "utilities/standard.h"
#include "Voxels.h"
#include "Materials.h"
#include "rendering/Mesh.h"
#include "physics/AABB.h"

#define CHUNKSIZE 16

enum class ChunkState {
    PENDING,
    READY,
    MESHED,
    LOADED,
};

// cubic chunk
class Chunk {
    static const int size = CHUNKSIZE;

public:
    Vec3 worldPosition;                          
    Voxel voxels[size * size * size];
    int heightMap[size][size];
    AABB box;
    Mesh mesh = Mesh();
    Mesh transparentMesh = Mesh();
    bool isDirty = true;
    ChunkState state = ChunkState::PENDING;

    Chunk(Vec3 position) : worldPosition(position) {
        box.min = worldPosition;
        box.max = worldPosition + (Vec3(1, 1, 1) * size);
    }

    ~Chunk() {}
    
    void addVoxel(const Vec3& localPosition, const Voxel& voxel);
    void removeVoxel(const Vec3& localPosition);
    Voxel* getVoxelAt(const Vec3& localPosition);

    Vec3 indexToPosition(int index) const;
    int positionToIndex(const Vec3& localPosition) const;

    bool positionIsSolid(const Vec3& localPosition) const;
    bool positionIsTransparent(const Vec3& localPosition) const;
    bool positionInBounds(const Vec3& localPosition) const;  

    void forEachVoxel(std::function<void(const Vec3&, const Voxel&)> callback) const;
    void forEachVoxel(std::function<void(const Vec3&, Voxel&)> callback);
};

class ChunkColumn {
    ChunkColumn(int chunkPosX, int chunkPosZ) 
        : chunkPosX(chunkPosX), chunkPosZ(chunkPosZ) {}

    static const int size = CHUNKSIZE;

    int chunkPosX, chunkPosZ;
    int heightMap[size * size];

    int getHeightAt(int localX, int localZ);
    int setHeightAt(int localX, int localZ);
    int updateHeightAt(int localX, int localZ);
};
