#pragma once

#include "WorldManager.h"
#include "utilities/PerlinNoise.h"
#include "utilities/ThreadManager.h"

class WorldGenerator {
public:
    WorldGenerator(WorldManager& worldManager, ThreadManager& threadManager) 
        : worldManager(worldManager), threadManager(threadManager) {}

    ~WorldGenerator() {}

    void setUpdateDistance(int updateDistance) { this->updateDistance = updateDistance; }

    void updateChunks(Vec3 worldCenter);

private:
    int updateDistance = 4;

    PerlinNoise perlin;
    PerlinNoise perlinHumid;
    PerlinNoise perlinHeight;
    PerlinNoise perlinTemp;

    WorldManager& worldManager;
    ThreadManager& threadManager;

    static const int chunkSize = CHUNKSIZE;

    void generateChunk(std::shared_ptr<Chunk> chunk);

    std::string generateBiome(Vec3 worldPosition, float height);

    float generateHeight(float x, float z);

    void generateChunk3D(std::shared_ptr<Chunk> chunk);
};
