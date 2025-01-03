#pragma once

#include "utilities/PerlinNoise.h"
#include "Chunk.h"

class ChunkGenerator {
public:
    ChunkGenerator() {}
    ~ChunkGenerator() {}

    void generateChunk(Chunk* chunk);
    
private:
    int updateDistance = 4;

    const int waterHeight = 75;

    PerlinNoise perlin;
    PerlinNoise perlinHumid;
    PerlinNoise perlinHeight;
    PerlinNoise perlinTemp;

    static const int chunkSize = SUBCHUNK_SIZE;

    std::string generateBiome(const Vec2& worldPosition2D, float height);

    float generateHeight(const Vec2& worldPosition2D);

    void generateChunk3D(Chunk* chunk);

    void generateSkyLight(Chunk* chunk);
};
