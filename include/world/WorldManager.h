#pragma once

#include "Chunk.h"

// Hash function for Vec3
struct Vec3Hash {
    std::size_t operator()(const Vec3& v) const {
        auto hashCombine = [](std::size_t seed, std::size_t value) {
            return seed ^ (value + 0x9e3779b9 + (seed << 6) + (seed >> 2));
        };

        int xInt = static_cast<int>(v.x * 1000);  // Scale to fixed precision
        int yInt = static_cast<int>(v.y * 1000);
        int zInt = static_cast<int>(v.z * 1000);

        std::size_t hash = std::hash<int>()(xInt);
        hash = hashCombine(hash, std::hash<int>()(yInt));
        hash = hashCombine(hash, std::hash<int>()(zInt));

        return hash;
    }
};

class WorldManager {
public:
    WorldManager(int chunkSize)
        : chunkSize(chunkSize) {}

    int getChunkSize() { return chunkSize; }

    // Chunk management
    void removeChunk(const Vec3& chunkPosition);
    std::shared_ptr<Chunk> addChunk(const Vec3& chunkPosition);
    std::shared_ptr<Chunk> getChunkAt(const Vec3& chunkPosition);
    bool chunkInCache(const Vec3& chunkPosition);

    // Static voxel operations
    bool addVoxel(const Vec3& worldPosition, const Voxel& voxel);
    bool removeVoxel(const Vec3& worldPosition);
    Voxel* getVoxel(const Vec3& worldPosition);
    bool voxelExistsAt(const Vec3& worldPosition);

    // Dynamic voxel management
    void addDynamicVoxel(const DynamicVoxel& voxel);
    void updateDynamicVoxels(float deltaTime); // Updates positions of all dynamic voxels
    const std::vector<DynamicVoxel>& getDynamicVoxels() const;

    // Utility functions
    Vec3 worldToChunkPosition(const Vec3& worldPosition) const;
    bool getFirstVoxelCollision(const Vec3& startPoint, const Vec3& endPoint, Vec3& voxelPos);

public:
    std::unordered_map<Vec3, std::shared_ptr<Chunk>, Vec3Hash> chunkCache;    
    std::vector<DynamicVoxel> dynamicVoxels;                                
    int chunkSize;                                                          
};