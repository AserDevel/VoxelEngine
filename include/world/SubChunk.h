#pragma once

#include "Voxels.h"
#include "rendering/Mesh.h"
#include <unordered_set>

#define SUBCHUNK_SIZE 16

class Chunk;

class SubChunk {
    static const int size = SUBCHUNK_SIZE;

    Voxel voxels[size * size * size];

    // Takes a local position and returns the index of the voxel if it exists within the subchunk
    int positionToIndex(const Vec3& localPosition) const;

public:
    const Vec3 worldPosition;

    Mesh mesh;
    Mesh transparentMesh;
    bool isDirty = true;

    SubChunk(Vec3 worldPosition) 
        : worldPosition(worldPosition) {}

    ~SubChunk() {}

    // voxel manipulation
    bool addVoxel(const Vec3& localPosition, const Voxel& newVoxel);
    bool removeVoxel(const Vec3& localPosition);    
    Voxel* getVoxelAt(const Vec3& localPosition);

    // Bounds checking
    bool positionInBounds(const Vec3& localPosition) const;
    bool positionIsEdge(const Vec3& localPosition) const; 

    // efficient voxel iteration with direct access to precomputed position and voxel
    void forEachVoxel(std::function<void(const Vec3&, const Voxel&)> callback) const;    
    void forEachVoxel(std::function<void(const Vec3&, Voxel&)> callback);
};