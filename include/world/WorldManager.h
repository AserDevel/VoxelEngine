#pragma once

#include "ChunkGenerator.h"
#include "ChunkMeshGenerator.h"
#include "LightGenerator.h"
#include "utilities/ThreadManager.h"

class WorldManager {
private:
    std::unordered_map<Vec2, std::unique_ptr<Chunk>, Vec2Hash> chunkCache;    
    const int chunkSize = SUBCHUNK_SIZE;
    int updateDistance;

    ChunkGenerator chunkGenerator;
    ChunkMeshGenerator meshGenerator;
    LightGenerator lightGenerator;
    ThreadManager& threadManager;

    // position converters
    Vec2 worldToChunkPosition(const Vec3& worldPosition) const;
    Vec2 worldToChunkPosition(const Vec2& worldPosition2D) const;    
    
    // utility functions
    bool neighboursReady(Chunk* chunk);
    void updateSkyLightAt(const Vec2 worldPosition2D);

    // Chunk management
    Chunk* addChunk(const Vec2& chunkPosition2D);
    Chunk* getChunk(const Vec2& chunkPosition2D) const;
    void removeChunk(const Vec2& chunkPosition2D);
    bool chunkInCache(const Vec2& chunkPosition2D);

public:
    WorldManager(ThreadManager& threadManager, int updateDistance)
        : updateDistance(updateDistance), threadManager(threadManager), meshGenerator(*this), lightGenerator(*this) {}

    // Updates all chunks in the a set range of the camera
    void updateChunks(Vec3 worldCenter);

    // Returns all chunks that are loaded and ready to be rendered
    std::vector<Chunk*> getLoadedChunks();
    void markDirty(const Vec3& worldPosition);

    // Global voxel operations
    void addVoxel(const Vec3& worldPosition, const Voxel& voxel);
    void removeVoxel(const Vec3& worldPosition);
    Voxel* getVoxelAt(const Vec3& worldPosition);

    // Position checking
    bool positionIsSolid(const Vec3& worldPosition);
    bool positionIsTransparent(const Vec3& worldPosition);
    uint8_t getLightLevelAt(const Vec3& worldPosition);

    // Physics
    bool worldRayDetection(const Vec3& startPoint, const Vec3& endPoint, Vec3& voxelPos, Vec3& normal);                                        
};