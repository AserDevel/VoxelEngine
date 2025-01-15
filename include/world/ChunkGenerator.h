#pragma once

#include "utilities/PerlinNoise.h"
#include "Chunk.h"
#include "Biomes.h"

class WorldManager;

class ChunkGenerator {
public:
    ChunkGenerator(WorldManager& worldManager) 
        : worldManager(worldManager) {
            std::random_device rd;
            unsigned int seed = rd();
            perlin = PerlinNoise(seed);
            rng = std::mt19937(seed);
        }

    ChunkGenerator(WorldManager& worldManager, unsigned int seed) 
        : worldManager(worldManager) {
            perlin = PerlinNoise(seed);
            rng = std::mt19937(seed);
        }

    ~ChunkGenerator() {}

    void generateChunk(Chunk* chunk);

    void generateFeatures(Chunk* chunk);
    
private:
    int updateDistance = 4;

    const int waterHeight = 112;
    
    WorldManager& worldManager;
    
    PerlinNoise perlin;
    std::mt19937 rng;

    static const int chunkSize = SUBCHUNK_SIZE;

    BiomeType getBiome(float height, float humid, float temp);

    float generateHeight(const Vec2& worldPosition2D);

    float generateWorldHeight(const Vec2& worldPosition2D, BiomeType biome, float height);

    Voxel generateVoxel(const Vec3& worldPositiom, BiomeType biome, int worldHeight);

    void generateTree(const Vec3& worldPosition);

    void generateChunk3D(Chunk* chunk);
};
