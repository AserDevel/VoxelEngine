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

    // Utility functions
    Vec2 worldToChunkPosition(const Vec3& worldPosition) const;    
    Vec3 chunkToWorldPosition(const Vec2& chunkPosition) const;

    // Chunk management
    void removeChunk(const Vec2& chunkPosition2D);
    Chunk* addChunk(const Vec2& chunkPosition2D);
    Chunk* getChunk(const Vec2& chunkPosition2D) const;
    bool chunkInCache(const Vec2& chunkPosition2D);

    bool neighboursReady(Chunk* chunk);

public:
    WorldManager(ThreadManager& threadManager, int updateDistance)
        : updateDistance(updateDistance), threadManager(threadManager), chunkGenerator(ChunkGenerator()) {}

    // Updates all chunks in the a set range of the camera
    void updateChunks(Vec3 worldCenter);

    void generateLighting(Chunk* chunk);

    void propagateLight(const Vec3& sourceWorldPosition, uint8_t initialLightLevel);

    // Returns all chunks that are loaded and ready to be rendered
    std::vector<Chunk*> getLoadedChunks();
    
    Chunk* getChunkAt(const Vec3& worldPosition) const;

    // Global voxel operations
    void addVoxel(const Vec3& worldPosition, const Voxel& voxel);
    void removeVoxel(const Vec3& worldPosition);
    Voxel* getVoxelAt(const Vec3& worldPosition);

    // Physics
    bool worldRayDetection(const Vec3& startPoint, const Vec3& endPoint, Vec3& voxelPos, Vec3& normal);                                        
};