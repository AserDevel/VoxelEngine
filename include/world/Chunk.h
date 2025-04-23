#pragma once

#include "Voxel.h"
#include "Biomes.h"
#include "rendering/Mesh.h"
#include <unordered_set>
#include <functional>

#define CHUNKSIZE 16

struct ColumnData {
    BiomeType biome;
    int worldHeight;
    float height;
    float humidity;
    float temperature;
};

class ChunkColumn {
    static const int size = CHUNKSIZE;

    ColumnData columnMap[size * size];

public:
    const Vec2 worldPosition2D;

    int dependencyCount = 0;

    ChunkColumn(const Vec2& worldPosition2D)
        : worldPosition2D(worldPosition2D) {}

    ColumnData getData(const Vec2& localPositon2D) {
        int index = localPositon2D.x + localPositon2D.z * size;
        return columnMap[index];
    }

    void setData(const Vec2& localPosition2D, ColumnData data) {
        int index = localPosition2D.x + localPosition2D.z * size;
        columnMap[index] = data;
    }
};

enum ChunkState {
    PENDING,
    GENERATED,
    DONE
};

class Chunk {
private:
    // Takes a local position and returns the index of the voxel if it exists within the subchunk
    int positionToIndex(const Vec3& localPosition) const;

public:
    static const int size = CHUNKSIZE;
    static const int numVoxels = size * size * size;
    
    const Vec3 worldPosition;
    const int bufferOffset;

    Voxel voxels[numVoxels];

    ChunkState state = PENDING;

    bool isEmpty = true;
    bool isDirty = true;

    Chunk(Vec3 worldPosition, int bufferOffset) 
        : worldPosition(worldPosition), bufferOffset(bufferOffset) {}

    ~Chunk() {}

    // voxel manipulation
    bool addVoxel(const Vec3& localPosition, const Voxel& newVoxel);
    bool removeVoxel(const Vec3& localPosition);    
    Voxel* getVoxel(const Vec3& localPosition);

    // Bounds checking
    bool positionInBounds(const Vec3& localPosition) const;
    bool positionIsEdge(const Vec3& localPosition) const; 

    // efficient voxel iteration with direct access to precomputed position and voxel
    void forEachVoxel(std::function<void(const Vec3&, const Voxel&)> callback) const;    
    void forEachVoxel(std::function<void(const Vec3&, Voxel&)> callback);
};