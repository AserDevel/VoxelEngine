#pragma once

#include "Voxel.h"
#include "rendering/Mesh.h"
#include <unordered_set>

#define CHUNKSIZE 16

class Chunk {
    // Takes a local position and returns the index of the voxel if it exists within the subchunk
    int positionToIndex(const Vec3& localPosition) const;

public:
    static const int size = CHUNKSIZE;
    static const int numVoxels = size * size * size;
    
    const Vec3 worldPosition;
    const int bufferOffset;

    Voxel voxels[numVoxels];

    bool isEmpty = true;
    bool isDirty = true;

    Chunk(Vec3 worldPosition, int bufferOffset) : worldPosition(worldPosition), bufferOffset(bufferOffset) {}

    ~Chunk() {}

    // voxel manipulation
    bool addVoxel(const Vec3& localPosition, const Voxel& newVoxel);
    bool removeVoxel(const Vec3& localPosition);    
    Voxel& getVoxel(const Vec3& localPosition);

    // Bounds checking
    bool positionInBounds(const Vec3& localPosition) const;
    bool positionIsEdge(const Vec3& localPosition) const; 

    // efficient voxel iteration with direct access to precomputed position and voxel
    void forEachVoxel(std::function<void(const Vec3&, const Voxel&)> callback) const;    
    void forEachVoxel(std::function<void(const Vec3&, Voxel&)> callback);
};