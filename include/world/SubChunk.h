#pragma once

#include "Voxels.h"
#include "Materials.h"
#include "rendering/Mesh.h"

#define SUBCHUNK_SIZE 16

class Chunk;

class SubChunk {
    static const int size = SUBCHUNK_SIZE;

    Voxel voxels[size * size * size];

    // Bounds checking
    bool positionInBounds(const Vec3& localPosition) const;

    // Takes a local position and returns the index of the voxel if it exists within the subchunk
    int positionToIndex(const Vec3& localPosition) const;

    int vertexAO(int i, int v, const Vec3& localVoxelPos);
    int calculateSkyLightAt(const Vec3& localPosition);

    // position checking
    bool positionIsSolid(const Vec3& localPosition);
    bool positionIsTransparent(const Vec3& localPosition);
    uint8_t getLightLevelAt(const Vec3& localPosition);

    // efficient voxel iteration with direct access to precomputed position and voxel
    void forEachVoxel(std::function<void(const Vec3&, const Voxel&)> callback) const;    
    void forEachVoxel(std::function<void(const Vec3&, Voxel&)> callback);

public:
    const Vec3 worldPosition;

    const Chunk* const parentChunk;

    Mesh mesh;
    Mesh transparentMesh;
    bool isDirty = true;

    SubChunk(Chunk* chunk, Vec3 worldPosition) 
        : worldPosition(worldPosition), parentChunk(chunk) {}

    ~SubChunk() {}

    // voxel manipulation
    bool addVoxel(const Vec3& localPosition, const Voxel& voxel);
    bool removeVoxel(const Vec3& localPosition);
    Voxel* getVoxelAt(const Vec3& localPosition);

    bool positionIsEdge(const Vec3& localPosition) const; 

    void updateLightSources();
    void updateLight();
    void generateMesh();
};