#pragma once

#include "utilities/PerlinNoise.h"
#include "Chunk.h"
#include "Biomes.h"

class WorldManager;

class ChunkGenerator {
private:
    const int waterHeight = 112;
    
    WorldManager& worldManager;
    
    PerlinNoise perlin;
    std::mt19937 rng;

    static const int chunkSize = CHUNKSIZE;

    // Generates a biome from 2D world position and height
    BiomeType getBiome(float height, float humid, float temp);

    // Generates height at the 2D world position
    float generateHeight(const Vec2& worldPosition2D);

    // Generates a voxel based on position, biome and height
    Voxel generateVoxel(const Vec3& worldPosition, BiomeType biome, int worldHeight);

    // Generates a tree
    void generateTree(const Vec3& worldPosition);

    // Generates caves
    void generateChunk3D(Chunk* chunk);

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

    void generateChunkColumn(ChunkColumn* column);
};
