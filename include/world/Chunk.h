#pragma once

#include "SubChunk.h"
#include "utilities/standard.h"
#include "physics/AABB.h"
#include <unordered_set>

#define MAX_HEIGHT 16 * SUBCHUNK_SIZE
#define MAX_DEPTH 0

enum class ChunkState {
    PENDING,
    GENERATED,
    MESHED,
    LOADED,
};

class WorldManager;

class Chunk {
private:
    static const int chunkSize = SUBCHUNK_SIZE;
    static const int maxHeight = MAX_HEIGHT;
    static const int maxDepth = MAX_DEPTH;

    const WorldManager* const worldManager;

    // flat 2D array containing the height of each column in the chunk         
    int heightMap[chunkSize * chunkSize];        

    // utility functions
    bool positionInBounds(const Vec3& worldPosition) const;
    int worldToChunkHeight(int worldHeight) const;

    // Subchunk management
    SubChunk* addSubChunk(int worldHeight);
    SubChunk* getSubChunk(int worldHeight) const;

    void updateHeightAt(const Vec2& localPosition2D);

    // mark affected subchunks when change at worldPosition
    void markDirtyNeighbours(const Vec3& worldPosition);

public:
    const Vec3 worldPosition; 

    // subchunks mapped vertically by chunkHeight 
    std::unordered_map<int, std::unique_ptr<SubChunk>> subChunks;  
        
    bool isDirty = true;
    ChunkState state = ChunkState::PENDING;

    Chunk(WorldManager* worldManager, Vec3 worldPosition) 
        : worldManager(worldManager), worldPosition(worldPosition) {}
    ~Chunk() {}

    SubChunk* getNeighbourAt(const Vec3& worldPosition) const;

    // voxel manipulation
    void addVoxel(const Vec3& worldPosition, const Voxel& voxel);
    void removeVoxel(const Vec3& worldPosition);
    Voxel* getVoxelAt(const Vec3& worldPosition);

    void markDirty(const Vec3& worldPosition);

    // height map manipulation
    int getHeightAt(const Vec2& localPosition2D) const;
    void setHeightAt(const Vec2& localPosition2D, int height);

    // recursive helper function to propagate light through the voxels within the chunk. Automatically spreads to neighbour chunks
    void propagateLight(const Vec3& worldPosition, uint8_t initialLightLevel, std::unordered_set<Vec3, Vec3Hash>& visited);
    void updateSkyLight();

    void generateMeshes();
    void loadMeshes();
    void draw();
    void drawTransparent();
};