#include "world/ChunkGenerator.h"

BiomeType ChunkGenerator::getBiome(float height, float humid, float temp) {
    BiomeType biome;
    if (height < 0.45) {
        biome = OCEAN;
    } else if (height > 0.6) {
        biome = MOUNTAINS;
    } else {
        if (humid > 0.5) {
            if (temp > 0.66) {
                biome = RAINFOREST;
            } else if (temp > 0.33) {
                biome = FOREST;
            } else {
                biome = TAIGA;
            }
        } else {
            if (temp > 0.66) {
                biome = DESSERT;
            } else if (temp > 0.33) {
                biome = PLAINS;
            } else {
                biome = SNOW;
            }
        }
    }

    return biome;
}

float ChunkGenerator::generateWorldHeight(const Vec2& worldPosition2D, BiomeType biome, float height) {
    int x = worldPosition2D.x, z = worldPosition2D.z;

    return height * 255;
}

Voxel ChunkGenerator::generateVoxel(const Vec3& worldPositiom, BiomeType biome, int worldHeight) {
    Voxel newVoxel;
    return newVoxel;
}

void ChunkGenerator::generateChunk(Chunk* chunk) {
    int wpx = chunk->worldPosition.x, wpz = chunk->worldPosition.z;
    for (int x = wpx; x < wpx + chunkSize; x++) {
        for (int z = wpz; z < wpz + chunkSize; z++) {
            // Generate base height
            float height = 0.5 + 0.2 * perlin.octaveNoise(x, z, 5, 0.5, 0.002);

            // Generate humidity
            float humid = 0.5 + 0.5 * perlin.octaveNoise(x, z, 3, 0.5, 0.002, Vec2(1000, 1000));

            // generate temperature
            float temp = 0.5 + 0.5 * perlin.octaveNoise(x, z, 3, 0.5, 0.0015, Vec2(-1000, -1000));

            BiomeType biome = getBiome(height, humid, temp);
            float blendingThreshold = 0.01;

            float mountainBlendFactor = 0.5 * (height + blendingThreshold - 0.6) / blendingThreshold;
            float humidBlendFactor = 0.5 * (humid + blendingThreshold - 0.5) / blendingThreshold;
            float warmBlendFactor = 0.5 * (temp + blendingThreshold - 0.66) / blendingThreshold;
            float coldBlendFactor = 0.5 * (temp + blendingThreshold - 0.33) / blendingThreshold;

            if (mountainBlendFactor >= 0) {
                float mountainThreshold = 0.5f;
                float frequency = 0.02;
                float mountainHeight = 0.3 * perlin.octaveNoise(x, z, 3, 0.5, 0.005, Vec2(-1000, 1000));
                //mountainHeight = height + smoothstep(mountainThreshold - 0.05f, mountainThreshold + 0.05f, mountainHeight); 
                if (mountainHeight < 0) mountainHeight = 0;
                mountainHeight = height + mountainHeight;
                height = mix(height, mountainHeight, mountainBlendFactor);
            }

            uint8_t lightLevel = 0x0F;
            int worldHeight = height * 255;
            int top = std::max(worldHeight, waterHeight);
            for (int y = top; y >= 0; y--) {     
                Voxel newVoxel = {0, 0, 0};
                switch (biome) {
                case OCEAN:
                    if (y > worldHeight) {
                        lightLevel = std::max(lightLevel - 1, 0);
                        newVoxel.lightLevel = lightLevel;
                        newVoxel.materialID = IDX_WATER;
                    } else if (y >= worldHeight - 1) {
                        newVoxel.materialID = IDX_SAND;
                    } else {
                        newVoxel.materialID = IDX_STONE;
                    } 
                    break;
                case MOUNTAINS:
                    newVoxel.materialID = IDX_STONE;
                    break;
                default:
                    if (y == worldHeight) {
                        newVoxel.materialID = IDX_GRASS;
                    } else if (y >= worldHeight -3) {
                        newVoxel.materialID = IDX_DIRT;
                    } else {
                        newVoxel.materialID = IDX_STONE;
                    }  
                    break;
                }
        
                chunk->addVoxel(Vec3(x, y, z), newVoxel);
            }
            chunk->setHeightAt(Vec2(x, z), worldHeight);
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
