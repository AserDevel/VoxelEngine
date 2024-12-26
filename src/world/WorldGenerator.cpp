#include "world/WorldGenerator.h"

void WorldGenerator::updateChunks(Vec3 worldCenter) {

    Vec3 centerChunkPosition = worldManager.worldToChunkPosition(worldCenter);

    for (int x = -updateDistance; x <= updateDistance; x++) {
        for (int y = -updateDistance; y <= updateDistance; y++) {
            for (int z = -updateDistance; z <= updateDistance; z++) {
                Vec3 chunkPosition = centerChunkPosition + Vec3(x, y, z);

                Vec2 worldCenter2D = Vec2(worldCenter.x, worldCenter.z);
                Vec2 chunkWorldCenter2D = Vec2(chunkPosition.x, chunkPosition.z) * chunkSize + Vec2(chunkSize / 2.0f, chunkSize / 2.0f);
                float distance = length(chunkWorldCenter2D - worldCenter2D);
                
                if (distance > updateDistance * chunkSize) {
                    if (worldManager.chunkInCache(chunkPosition)) {
                        worldManager.removeChunk(chunkPosition); // Remove chunk from cache
                    } 
                    continue;
                } 

                // Create a batch with every chunk that needs to generate
                if (!worldManager.chunkInCache(chunkPosition)) {
                    auto chunk = worldManager.addChunk(chunkPosition);
                    chunk->state = ChunkState::PENDING;    
                    
                    // add generation task to the queue
                    threadManager.addTask([this, chunk]() {
                        generateChunk(chunk);
                    });
                }
            }  
        }
    }
}

float WorldGenerator::generateHeight(float x, float z) {
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

    height += mountainHeight;
    
    return height;
}


std::string WorldGenerator::generateBiome(Vec3 worldPosition, float height) {
    float biomeFrequency = 0.001f;
    float temp = perlinTemp.noise(worldPosition.x * biomeFrequency, worldPosition.z * biomeFrequency) * 0.5f + 0.5f; // [0,1]
    float humid = perlinHumid.noise(worldPosition.x * biomeFrequency, worldPosition.z * biomeFrequency) * 0.5f + 0.5f; // [0,1]

    std::string res;

    if (height <= 0) {
        if (temp > 0.33) return "ocean";
        else return "coldOcean";
    } else if (height > 60) {
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

void WorldGenerator::generateChunk(std::shared_ptr<Chunk> chunk) {
    Vec3 wp = chunk->worldPosition;
    if (wp.y < -100.0f) return; // skip chunks that are in the void

    for (int x = 0; x < chunkSize; x++) {
        for (int z = 0; z < chunkSize; z++) {
            int height = generateHeight(wp.x + x, wp.z + z);
            chunk->heightMap[x][z] = height;
            std::string biome = generateBiome(wp, height);
            Voxel newVoxel;
            
            if (biome == "dessert") {
                for (int y = 0; wp.y + y < height && y < chunkSize; y++) {
                    if (wp.y + y > 0) {
                        newVoxel.materialID = IDX_SAND;
                    } else {
                        newVoxel.materialID = IDX_STONE;
                    }
                    chunk->addVoxel(Vec3(x, y, z), newVoxel);
                }
                continue;
            }

            if (biome == "ocean" || biome == "coldOcean") {
                for (int y = 0; y < chunkSize; y++) {
                    if (wp.y + y <= 0) {
                        if (wp.y + y == height - 1) {
                            newVoxel.materialID = IDX_SAND;
                            int skyLight = clamp(15 + height, 0, 15);
                            //newVoxel.metaData |= (skyLight << OFFSET_SKYLIGHT);
                        } else if (wp.y + y < height) {
                            newVoxel.materialID = IDX_STONE;
                        } else {
                            newVoxel.materialID = IDX_WATER;
                        }
                        chunk->addVoxel(Vec3(x, y, z), newVoxel);
                    }
                } 
                continue;
            }
            
            for (int y = 0; wp.y + y < height && y < chunkSize; y++) {
                if (wp.y + y == height - 1) {
                    newVoxel.materialID = IDX_GRASS;
                } else if (wp.y + y > height -3) {
                    newVoxel.materialID = IDX_DIRT;
                } else {
                    newVoxel.materialID = IDX_STONE;
                }
                chunk->addVoxel(Vec3(x, y, z), newVoxel);   
            }
        }
    }

    chunk->state = ChunkState::READY;
}

void WorldGenerator::generateChunk3D(std::shared_ptr<Chunk> chunk) {
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
