#pragma once

#include "WorldManager.h"
#include "utilities/PerlinNoise.h"
#include "utilities/ThreadManager.h"

class WorldGenerator {
public:
    WorldGenerator(WorldManager& worldManager, ThreadManager& threadManager) : worldManager(worldManager), threadManager(threadManager) {}

    ~WorldGenerator() {}

    void setUpdateDistance(int updateDistance) { this->updateDistance = updateDistance; }

    void updateChunks(Vec3 centerWorldPosition);

private:
    int updateDistance = 4;
    uint64_t seed;  
    PerlinNoise perlin;

    WorldManager& worldManager;
    ThreadManager& threadManager;

    void generateChunk(std::shared_ptr<Chunk> chunk);
};
