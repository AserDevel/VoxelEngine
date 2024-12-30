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
    static const int chunkSize = SUBCHUNK_SIZE;
    static const int maxHeight = MAX_HEIGHT;
    static const int maxDepth = MAX_DEPTH;

    const WorldManager* const worldManager;

    // flat 2D array containing the height of each column in the chunk         
    int heightMap[chunkSize * chunkSize];

    // subchunks mapped vertically by chunkHeight 
    std::unordered_map<int, std::unique_ptr<SubChunk>> subChunks;          

    // utility functions
    bool positionInBounds(const Vec3& worldPosition) const;
    int worldToChunkHeight(int worldHeight) const;

    // Subchunk management
    SubChunk* addSubChunkAt(int worldHeight);
    void removeSubChunk(int worldHeight);

public:
    const Vec3 worldPosition; 
    const Vec2 chunkPosition2D;      
        
    bool isDirty = true;
    ChunkState state = ChunkState::PENDING;

    Chunk(WorldManager* worldManager, Vec2 chunkPosition2D) :
        worldManager(worldManager),
        chunkPosition2D(chunkPosition2D),
        worldPosition(Vec3(chunkPosition2D.x * chunkSize, maxDepth, chunkPosition2D.z * chunkSize)) {}

    ~Chunk() {}

    // returns the subchunk within the given world height. Also checks the neighbour chunks. initializes a new subchunk on first call
    SubChunk* getSubChunkAt(int worldHeight) const;

    // voxel manipulation
    bool addVoxel(const Vec3& worldPosition, const Voxel& voxel);
    bool removeVoxel(const Vec3& worldPosition);

    // position checking
    bool positionIsSolid(const Vec3& worldPosition) const;
    bool positionIsTransparent(const Vec3& worldPosition) const;

    // mark affected subchunks when change at worldPosition
    void markDirty(const Vec3& worldPosition);

    // height map manipulation
    int getHeightAt(const Vec2& worldPosition2D) const;
    void setHeightAt(const Vec2& worldPosition2D, int height);

    // recursive helper function to propagate light through the voxels within the chunk. Automatically spreads to neighbour chunks
    void propagateLight(const Vec3& sourceWorldPosition, uint8_t initialLightLevel, std::unordered_set<Vec3, Vec3Hash>& visited);

    void generateMeshes();
    void loadMeshes();
    void draw();
    void drawTransparent();
};