#pragma once

#include "WorldManager.h"
#include "PerlinNoise.h"
#include "world/ThreadManager.h"

class WorldGenerator {
public:
    WorldGenerator(WorldManager& worldManager, ThreadManager& threadManager) : worldManager(worldManager), threadManager(threadManager) {}

    ~WorldGenerator() {}

    void setUpdateDistance(int updateDistance) { this->updateDistance = updateDistance; }

    void updateChunks(Vec3 centerChunk);

private:
    int updateDistance = 4;
    uint64_t seed;  
    PerlinNoise perlin;

    WorldManager& worldManager;
    ThreadManager& threadManager;

    void generateChunk(std::shared_ptr<Chunk> chunk, Vec3 worldPosition);
};
