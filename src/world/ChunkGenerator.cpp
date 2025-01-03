#include "world/ChunkGenerator.h"

float ChunkGenerator::generateHeight(const Vec2& worldPosition2D) {
    int x = worldPosition2D.x, z = worldPosition2D.z;
    
    float frequency = 1.0f / 16;
    float amplitude = 4.0f;

    // general heightmap
    float height = 100.0f * perlinHeight.noise(x * 0.002f, z * 0.002f);

    // generate more detailed terrain
    for (int i = 0; i < 3; i++) {
        height += amplitude * perlinHeight.noise(x * frequency, z * frequency);
        frequency *= 0.5f;
        amplitude *= 2.0f;
    }

    // generate steep mountains and cliffs
    float mountainThreshold = 0.3f;
    frequency = 0.1f / 16;
    amplitude = 30.0f;
    float mountainHeight = perlinHeight.noise(x * frequency, z * frequency);
    mountainHeight = amplitude * smoothstep(mountainThreshold - 0.2f, mountainThreshold + 0.2f, mountainHeight);

    /*
    float cliffThreshold = 0.5f;
    frequency = 1.0f / 16;
    amplitude = 10.0f;
    float cliffHeight = perlinHeight.noise(x * frequency, z * frequency);
    cliffHeight = amplitude * smoothstep(cliffThreshold - 0.05f, cliffThreshold + 0.05f, cliffHeight);
    */

    height += mountainHeight + 100;
    
    return height;
}


std::string ChunkGenerator::generateBiome(const Vec2& worldPosition2D, float height) {
    float biomeFrequency = 0.001f;
    float temp = perlinTemp.noise(worldPosition2D.x * biomeFrequency, worldPosition2D.z * biomeFrequency) * 0.5f + 0.5f; // [0,1]
    float humid = perlinHumid.noise(worldPosition2D.x * biomeFrequency, worldPosition2D.z * biomeFrequency) * 0.5f + 0.5f; // [0,1]

    std::string res;

    if (height < waterHeight) {
        if (temp > 0.33) return "ocean";
        else return "coldOcean";
    } else if (height > 100) {
        if (temp > 0.33) return "mountains";
        else return "snowyMountains";
    }

    if (temp > 0.7) {
        if (humid > 0.5) res = "rainForest";
        else res = "dessert";
    } else if (temp > 0.3) {
        if (humid > 0.5) res = "forest";
        else res = "grass";
    } else {
        if (humid > 0.5) res = "taiga";
        else res = "snow";
    }

    return res;
}

void ChunkGenerator::generateChunk(Chunk* chunk) {
    Vec2 worldPosition2D = Vec2(chunk->worldPosition.x, chunk->worldPosition.z);
    
    for (int x = 0; x < chunkSize; x++) {
        for (int z = 0; z < chunkSize; z++) {
            const Vec2 wp = worldPosition2D + Vec2(x, z);
            
            int height = generateHeight(wp);
            chunk->setHeightAt(Vec2(x, z), height);

            std::string biome = generateBiome(wp, height);

            Voxel newVoxel;
            for (int y = 0; y <= height || y < waterHeight; y++) {                
                if (biome == "ocean" || biome == "coldOcean") {
                    if (y == height) {
                        newVoxel.materialID = IDX_SAND;
                    } else if (y < height) {
                        newVoxel.materialID = IDX_STONE;
                    } else {
                        newVoxel.materialID = IDX_WATER;
                    }
                    
                    chunk->addVoxel(Vec3(wp.x, y, wp.z), newVoxel);
                    continue;
                }

                if (y == height) {
                    newVoxel.materialID = IDX_GRASS;
                } else if (y > height -3) {
                    newVoxel.materialID = IDX_DIRT;
                } else {
                    newVoxel.materialID = IDX_STONE;
                }
        
                chunk->addVoxel(Vec3(wp.x, y, wp.z), newVoxel);
            }
        }
    }
    
    chunk->state = ChunkState::GENERATED;
}

void ChunkGenerator::generateChunk3D(Chunk* chunk) {
    Vec3 wp = chunk->worldPosition;
    Voxel newVoxel;
    for (double x = 0; x < chunkSize; x++) {
        for (double y = 0; y < chunkSize; y++) {
            for (double z = 0; z < chunkSize; z++) {
                double val = perlin.noise((wp.x + x) / chunkSize, (wp.y + y) / chunkSize, (wp.z + z) / chunkSize);
                if (val > 0.3) chunk->addVoxel(Vec3(x, y, z), {5,0}); 
            }
        }
    }
}
