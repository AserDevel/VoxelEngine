#pragma once

#include "ChunkGenerator.h"
#include "utilities/ThreadManager.h"

class WorldManager {
private:
    std::unordered_map<Vec2, std::unique_ptr<Chunk>, Vec2Hash> chunkCache;    
    const int chunkSize = SUBCHUNK_SIZE;
    int updateDistance;

    ChunkGenerator chunkGenerator;

    ThreadManager& threadManager;

    // Chunk management
    void removeChunk(const Vec2& chunkPosition2D);
    Chunk* addChunk(const Vec2& chunkPosition2D);
    bool chunkInCache(const Vec2& chunkPosition2D);

    bool neighboursReady(Chunk* chunk);

public:
    WorldManager(ThreadManager& threadManager, int updateDistance)
        : updateDistance(updateDistance), threadManager(threadManager), chunkGenerator(ChunkGenerator()) {}

    // Updates all chunks in the a set range of the camera
    void updateChunks(Vec3 worldCenter);

    // Returns all chunks that are loaded and ready to be rendered
    std::vector<Chunk*> getLoadedChunks();
    
    Chunk* getChunkAt(const Vec2& chunkPosition2D) const;

    // Global voxel operations
    void addVoxel(const Vec3& worldPosition, const Voxel& voxel);
    void removeVoxel(const Vec3& worldPosition);

    // Global position checking
    bool positionIsSolid(const Vec3& worldPosition) const;
    bool positionIsTransparent(const Vec3& worldPosition) const;

    // Utility functions
    Vec2 worldToChunkPosition(const Vec3& worldPosition) const;    

    // Physics
    bool worldRayDetection(const Vec3& startPoint, const Vec3& endPoint, Vec3& voxelPos, Vec3& normal);                                        
};