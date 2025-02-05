#include "world/ChunkGenerator.h"
#include "world/WorldManager.h"

void ChunkGenerator::generateChunkColumn(ChunkColumn* column) {
    for (int x = 0; x < chunkSize; x++) {
        for (int z = 0; z < chunkSize; z++) {
            Vec2 wp2D = column->worldPosition2D + Vec2(x, z);
            // Generate height
            float height = generateHeight(wp2D);
            int worldHeight = height * 255;

            // Generate humidity
            float humid = 0.5 + 0.5 * perlin.octaveNoise(wp2D.x, wp2D.z, 3, 0.5, 0.002, Vec2(1000, 1000));

            // Generate temperature
            float temp = 0.5 + 0.5 * perlin.octaveNoise(wp2D.x, wp2D.z, 3, 0.5, 0.0015, Vec2(-1000, -1000));
            BiomeType biome = getBiome(height, humid, temp);
            
            ColumnData data;
            data.biome = biome;
            data.worldHeight = worldHeight;
            data.height = height;
            data.humidity = humid;
            data.temperature = temp;

            column->setData(Vec2(x, z), data);
        }
    }
}

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

Voxel ChunkGenerator::generateVoxel(const Vec3& worldPosition, BiomeType biome, int worldHeight) {
    std::uniform_real_distribution uniformDist(0.0, 1.0);
    int y = worldPosition.y;
    Voxel newVoxel = 0;
    if (y > worldHeight) return newVoxel;

    switch (biome) {
    case MOUNTAINS:
        newVoxel.setMatID(ID_STONE);
        break;
    case DESSERT:
        if (y >= worldHeight - 8) {
            newVoxel.setMatID(ID_SAND);
        } else {
            newVoxel.setMatID(ID_STONE);
        }
        break;
    default:
        if (y == worldHeight) {
            newVoxel.setMatID(ID_GRASS);
        } else if (y >= worldHeight -3) {
            newVoxel.setMatID(ID_DIRT);
        } else {
            newVoxel.setMatID(ID_STONE);
        }  
        break;
    }

    return newVoxel;
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

void ChunkGenerator::generateTree(const Vec3& worldPosition) {
    const Vec3 trunk[6] = { Vec3(0, 5, 0), Vec3(0, 4, 0), Vec3(0, 3, 0), Vec3(0, 2, 0), Vec3(0, 1, 0), Vec3(0, 0, 0) };
    const Vec3 crown[17] {
        Vec3(0, 6, 0),
        Vec3(1, 5, 0), Vec3(-1, 5, 0), Vec3(0, 5, 1), Vec3(0, 5, -1),
        Vec3(1, 4, 0), Vec3(-1, 4, 0), Vec3(0, 4, 1), Vec3(0, 4, -1),
        Vec3(1, 3, 0), Vec3(-1, 3, 0), Vec3(0, 3, 1), Vec3(0, 3, -1),
        Vec3(1, 2, 0), Vec3(-1, 2, 0), Vec3(0, 2, 1), Vec3(0, 2, -1),
    };

    Voxel wood = ID_WOOD;
    for (const Vec3& pos : trunk) {
        worldManager.addVoxel(worldPosition + pos, wood);
    }

    Voxel leaves = ID_LEAVES;
    for (const Vec3& pos : crown) {
        worldManager.addVoxel(worldPosition + pos, leaves);
    }   
}

void ChunkGenerator::generateFeatures(Chunk* chunk) {
    std::uniform_int_distribution uniformDist(1, 30);
    Vec2 columnPos = floor(chunk->worldPosition.xz() / chunkSize);
    ChunkColumn* column = worldManager.getColumn(columnPos);
    for (int x = 0; x < chunkSize; x++) {
        for (int z = 0; z < chunkSize; z++) {
            int height = column->getData(Vec2(x, z)).worldHeight;
            if (height >= chunk->worldPosition.y && height < chunk->worldPosition.y + chunkSize) {
                Vec3 wp = Vec3(chunk->worldPosition.x + x, height, chunk->worldPosition.z + z);
                Voxel* voxel = worldManager.getVoxel(wp);
                if (voxel && voxel->getMatID() == ID_GRASS) {
                    if (uniformDist(rng) == 1) generateTree(wp + Vec3(0, 1, 0));
                }
            }            
        }
    }
    chunk->state = DONE;
}

void ChunkGenerator::generateChunk(Chunk* chunk) {
    std::uniform_real_distribution uniformDist(0.0, 1.0);
    Vec2 columnPos = floor(chunk->worldPosition.xz() / chunkSize);
    ChunkColumn* column = worldManager.getColumn(columnPos);
    for (int x = 0; x < chunkSize; x++) {
        for (int z = 0; z < chunkSize; z++) {
            int wpx = chunk->worldPosition.x + x, wpz = chunk->worldPosition.z + z;
            
            ColumnData data = column->getData(Vec2(x, z));
            BiomeType biome = data.biome;
            int worldHeight = data.worldHeight;
            float height = data.height;
            float humid = data.humidity;
            float temp = data.temperature;

            float blendingThreshold = 0.02;
            float mountainBlendFactor = 0.5 * (height + blendingThreshold - 0.6) / blendingThreshold;
            float humidBlendFactor = 0.5 * (humid + blendingThreshold - 0.5) / blendingThreshold;
            float warmBlendFactor = 0.5 * (temp + blendingThreshold - 0.66) / blendingThreshold;
            float coldBlendFactor = 0.5 * (temp + blendingThreshold - 0.33) / blendingThreshold;
            
            for (int y = 0; y < chunkSize; y++) {     
                int wpy = chunk->worldPosition.y + y;
                Voxel newVoxel = 0;
                if (wpy > std::max(worldHeight, waterHeight)) break;
                chunk->isEmpty = false;
                
                if (biome == OCEAN) {
                    if (wpy > worldHeight) {
                        newVoxel.setMatID(ID_WATER);
                    } else if (wpy >= worldHeight - 1) {
                        newVoxel.setMatID(ID_SAND);
                    } else {
                        newVoxel.setMatID(ID_STONE);
                    }
                } else {
                    Vec3 wp = Vec3(wpx, wpy, wpz);
                    newVoxel = generateVoxel(wp, biome, worldHeight);
                    
                    if (mountainBlendFactor >= 0.5 && mountainBlendFactor <= 1.0) {
                        BiomeType blendBiome = getBiome(height - blendingThreshold, humid, temp);
                        Voxel blendVoxel = generateVoxel(wp, blendBiome, worldHeight);
                        newVoxel = mountainBlendFactor > uniformDist(rng) ? newVoxel : blendVoxel;
                    } else if (mountainBlendFactor >= 0 && mountainBlendFactor < 0.5) {
                        BiomeType blendBiome = MOUNTAINS;
                        Voxel blendVoxel = generateVoxel(wp, blendBiome, worldHeight);
                        newVoxel = mountainBlendFactor > uniformDist(rng) ? blendVoxel : newVoxel;
                    }
                }
                
                chunk->addVoxel(Vec3(x, y, z), newVoxel);
            }
        }
    }
    generateChunk3D(chunk);   

    chunk->state = GENERATED; 
}

void ChunkGenerator::generateChunk3D(Chunk* chunk) {
    Vec3 wp = chunk->worldPosition;
    Voxel newVoxel;
    for (double x = wp.x; x < wp.x + chunkSize; x++) {
        for (double y = wp.y; y < wp.y + chunkSize; y++) {
            for (double z = wp.z; z < wp.z + chunkSize; z++) {
                double val = perlin.noise(x / chunkSize, y / chunkSize, z / chunkSize);
                if (val < -0.4) worldManager.removeVoxel(Vec3(x, y, z)); 
            }
        }
    }
}
