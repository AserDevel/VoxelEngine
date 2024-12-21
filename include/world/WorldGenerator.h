#pragma once

#include "WorldManager.h"
#include "utilities/PerlinNoise.h"
#include "utilities/ThreadManager.h"

class WorldGenerator {
public:
    WorldGenerator(WorldManager& worldManager, ThreadManager& threadManager) : worldManager(worldManager), threadManager(threadManager) {
        chunkSize = worldManager.getChunkSize();
    }

    ~WorldGenerator() {}

    void setUpdateDistance(int updateDistance) { this->updateDistance = updateDistance; }

    void updateChunks(Vec3 centerWorldPosition);

private:
    int updateDistance = 4;

    PerlinNoise perlin;
    PerlinNoise perlinHumid;
    PerlinNoise perlinHeight;
    PerlinNoise perlinTemp;

    WorldManager& worldManager;
    ThreadManager& threadManager;

    int chunkSize = 16;

    void generateChunk(std::shared_ptr<Chunk> chunk);

    std::string generateBiome(Vec3 worldPosition, float height);

    float generateHeight(float x, float z);

    void generateChunk3D(std::shared_ptr<Chunk> chunk);
};
