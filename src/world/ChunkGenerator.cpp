#include "world/ChunkGenerator.h"
#include "world/WorldManager.h"

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
    std::uniform_real_distribution uniformDist(0.0, 1.0);
    int y = worldPositiom.y;
    Voxel newVoxel = {0, 0, 0};

    switch (biome) {
    case MOUNTAINS:
        newVoxel.materialID = IDX_STONE;
        break;
    case DESSERT:
        if (y >= worldHeight - 8) {
            newVoxel.materialID = IDX_SAND;
        } else {
            newVoxel.materialID = IDX_STONE;
        }
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

    return newVoxel;
}

void ChunkGenerator::generateTree(const Vec3& worldPosition) {
    const Vec3 trunk[6] = { Vec3(0, 5, 0), Vec3(0, 4, 0), Vec3(0, 3, 0), Vec3(0, 2, 0), Vec3(0, 1, 0), Vec3(0, 0, 0) };
    const Vec3 crown[17] {
        Vec3(0, 6, 0),
        Vec3(1, 5, 0), Vec3(-1, 5, 0), Vec3(0, 5, 1), Vec3(0, 5, -1),
        Vec3(1, 4, 0), Vec3(-1, 4, 0), Vec3(0, 4, 1), Vec3(0, 4, -1),
        Vec3(1, 3, 0), Vec3(-1, 3, 0), Vec3(0, 3, 1), Vec3(0, 3, -1),
        Vec3(1, 2, 0), Vec3(-1, 2, 0), Vec3(0, 2, 1), Vec3(0, 2, -1),
    };

    Voxel wood = {IDX_WOOD, 0, 0};
    for (const Vec3& pos : trunk) {
        worldManager.addVoxel(worldPosition + pos, wood);
    }

    Voxel leaves = {IDX_LEAVES, 0, 0};
    for (const Vec3& pos : crown) {
        worldManager.addVoxel(worldPosition + pos, leaves);
    }   
}

float ChunkGenerator::generateHeight(const Vec2& worldPosition2D) {
    int x = worldPosition2D.x, z = worldPosition2D.z;

    float height = 0.5 + 0.2 * perlin.octaveNoise(x, z, 5, 0.5, 0.002);

    float mountainThreshold = 0.02f;
    float mountainBlending = 0.5 * (height + mountainThreshold - 0.6) / mountainThreshold;
    if (mountainBlending >= 0.0) {
        float mountainHeight = 0.5 * perlin.octaveNoise(x, z, 4, 0.5, 0.005, Vec2(-1000, 1000));
        mountainHeight = height + height * mountainHeight;
        height = mix(height, mountainHeight, mountainBlending);
    }
    
    return height;
}


void ChunkGenerator::generateFeatures(Chunk* chunk) {
    std::uniform_int_distribution uniformDist(1, 100);
    int wpx = chunk->worldPosition.x, wpz = chunk->worldPosition.z;
    for (int x = wpx; x < wpx + chunkSize; x++) {
        for (int z = wpz; z < wpz + chunkSize; z++) {
            int y = chunk->getHeightAt(Vec2(x, z));

            if (chunk->getVoxelAt(Vec3(x, y, z))->materialID == IDX_GRASS) {
                if (uniformDist(rng) == 1) generateTree(Vec3(x, y+1, z));
            }
        }
    }

    chunk->state = ChunkState::READY;
}

void ChunkGenerator::generateChunk(Chunk* chunk) {
    std::uniform_real_distribution uniformDist(0.0, 1.0);
    int wpx = chunk->worldPosition.x, wpz = chunk->worldPosition.z;
    for (int x = wpx; x < wpx + chunkSize; x++) {
        for (int z = wpz; z < wpz + chunkSize; z++) {
            // Generate base height
            float height = generateHeight(Vec2(x, z));

            // Generate humidity
            float humid = 0.5 + 0.5 * perlin.octaveNoise(x, z, 3, 0.5, 0.002, Vec2(1000, 1000));

            // generate temperature
            float temp = 0.5 + 0.5 * perlin.octaveNoise(x, z, 3, 0.5, 0.0015, Vec2(-1000, -1000));

            BiomeType biome = getBiome(height, humid, temp);
            
            float blendingThreshold = 0.02;
            
            float mountainBlendFactor = 0.5 * (height + blendingThreshold - 0.6) / blendingThreshold;
            float humidBlendFactor = 0.5 * (humid + blendingThreshold - 0.5) / blendingThreshold;
            float warmBlendFactor = 0.5 * (temp + blendingThreshold - 0.66) / blendingThreshold;
            float coldBlendFactor = 0.5 * (temp + blendingThreshold - 0.33) / blendingThreshold;

            uint8_t lightLevel = 0x0F;
            int worldHeight = height * 255;
            int top = std::max(worldHeight, waterHeight);
            for (int y = top; y >= 0; y--) {     
                Voxel newVoxel = {0, 0, 0};
                if (biome == OCEAN) {
                    if (y > worldHeight) {
                        lightLevel = std::max(lightLevel - 1, 0);
                        newVoxel.lightLevel = lightLevel;
                        newVoxel.materialID = IDX_WATER;
                    } else if (y >= worldHeight - 1) {
                        newVoxel.materialID = IDX_SAND;
                    } else {
                        newVoxel.materialID = IDX_STONE;
                    }
                } else {
                    newVoxel = generateVoxel(Vec3(x, y, z), biome, worldHeight);
                    if (mountainBlendFactor >= 0.5 && mountainBlendFactor <= 1.0) {
                        BiomeType blendBiome = getBiome(height - blendingThreshold, humid, temp);
                        Voxel blendVoxel = generateVoxel(Vec3(x, y, z), blendBiome, worldHeight);
                        newVoxel = mountainBlendFactor > uniformDist(rng) ? newVoxel : blendVoxel;
                    } else if (mountainBlendFactor >= 0 && mountainBlendFactor < 0.5) {
                        BiomeType blendBiome = MOUNTAINS;
                        Voxel blendVoxel = generateVoxel(Vec3(x, y, z), blendBiome, worldHeight);
                        newVoxel = mountainBlendFactor > uniformDist(rng) ? blendVoxel : newVoxel;
                    }
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
