#pragma once

#include "SubChunk.h"
#include "utilities/standard.h"
#include "physics/AABB.h"
#include <unordered_set>

#define MAX_HEIGHT 16 * SUBCHUNK_SIZE - 1
#define MAX_DEPTH 0

enum class ChunkState {
    PENDING,
    GENERATED,
    MESHED,
    LOADED,
};

class Chunk {
private:
    static const int chunkSize = SUBCHUNK_SIZE;
    static const int maxHeight = MAX_HEIGHT;
    static const int maxDepth = MAX_DEPTH;

    // flat 2D array containing the height of each column in the chunk         
    int heightMap[chunkSize * chunkSize] = {MAX_DEPTH};

    int worldToChunkHeight(int worldHeight) const;

public:
    const Vec3 worldPosition; 
    bool isDirty = true;

    // subchunks mapped vertically by chunkHeight 
    std::unordered_map<int, std::unique_ptr<SubChunk>> subChunks;  
        
    ChunkState state = ChunkState::PENDING;

    Chunk(Vec3 worldPosition) 
        : worldPosition(worldPosition) {}
    ~Chunk() {}

    // Subchunk management
    SubChunk* addSubChunk(int worldHeight);
    SubChunk* getSubChunk(int worldHeight) const;

    void markDirty(const Vec3& worldPosition);

    // bounds Checking
    bool positionInBounds(const Vec3& worldPosition) const;
    bool postitionIsEdge(const Vec3& worldPosition) const;

    // voxel manipulation
    bool addVoxel(const Vec3& worldPosition, const Voxel& voxel);
    bool removeVoxel(const Vec3& worldPosition);
    Voxel* getVoxelAt(const Vec3& worldPosition);

    // height map manipulation
    int getHeightAt(const Vec2& worldPosition2D) const;
    void setHeightAt(const Vec2& worldPosition2D, int height);
    void updateHeightAt(const Vec2& worldPosition2D);

    void loadMeshes();
    void draw();
    void drawTransparent();
};