#pragma once

#include "utilities/PerlinNoise.h"
#include "Chunk.h"
#include "Biomes.h"

class ChunkGenerator {
public:
    ChunkGenerator() {}
    ~ChunkGenerator() {}

    void generateChunk(Chunk* chunk);
    
private:
    int updateDistance = 4;

    const int waterHeight = 112;

    PerlinNoise perlin;
    PerlinNoise perlinHumid;
    PerlinNoise perlinHeight;
    PerlinNoise perlinTemp;

    static const int chunkSize = SUBCHUNK_SIZE;

    BiomeType getBiome(float height, float humid, float temp);

    float generateWorldHeight(const Vec2& worldPosition2D, BiomeType biome, float height);

    Voxel generateVoxel(const Vec3& worldPositiom, BiomeType biome, int worldHeight);

    void generateChunk3D(Chunk* chunk);
};
