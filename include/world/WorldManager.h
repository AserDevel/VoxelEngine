#pragma once

#include "ChunkGenerator.h"
#include "physics/AABB.h"
#include "utilities/ThreadManager.h"

class WorldManager {
private:
    const int chunkSize = CHUNKSIZE;

    std::unordered_map<Vec3, std::unique_ptr<Chunk>, Vec3Hash> activeChunks;   
    std::unordered_map<Vec2, std::unique_ptr<ChunkColumn>, Vec2Hash> activeColumns;
    std::queue<int> availableOffsets;

    ChunkGenerator chunkGenerator;
    ThreadManager& threadManager;

    // position converters
    Vec3 worldToChunkPosition(const Vec3& worldPosition) const;
    
    // Chunk management
    Chunk* addChunk(const Vec3& chunkPosition, int bufferOffset);
    Chunk* getChunk(const Vec3& chunkPosition) const;
    ChunkColumn* addColumn(Vec2 columnPosition);
    bool neighboursReady(Chunk* chunk);

public:
    int updateDistance = 4;
    int numChunks;

    Chunk** chunks; //flatened array where chunks are mapped with relative chunk positions 
    
    WorldManager(ThreadManager& threadManager, int updateDistance)
        : updateDistance(updateDistance), threadManager(threadManager), chunkGenerator(*this) {
        int worldEdgeLen = updateDistance * 2 + 1;
        numChunks = worldEdgeLen * worldEdgeLen * worldEdgeLen;
        chunks = new Chunk*[numChunks]();
        int numVoxels = chunkSize * chunkSize * chunkSize;
        for (int i = 0; i < numChunks; i++) {
            int offset = i * numVoxels;
            availableOffsets.push(offset);
        }
    }

    // Updates all chunks in the a set range of the camera
    void updateChunks(Vec3 worldCenter);

    // Get column at the column position. Returns null if invalid
    ChunkColumn* getColumn(Vec2 columnPosition) const;

    // Global voxel operations
    void addVoxel(const Vec3& worldPosition, const Voxel& voxel);
    void removeVoxel(const Vec3& worldPosition);
    Voxel* getVoxel(const Vec3& worldPosition);

    // Position checking
    bool positionIsSolid(const Vec3& worldPosition);
    bool positionIsTransparent(const Vec3& worldPosition);

    // Physics
    bool worldRayDetection(const Vec3& startPoint, const Vec3& endPoint, Vec3& voxelPos, Vec3& normal);                                        
};