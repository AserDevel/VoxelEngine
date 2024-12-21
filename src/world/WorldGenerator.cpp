#include "world/WorldGenerator.h"

void WorldGenerator::updateChunks(Vec3 centerWorldPosition) {

    Vec3 centerChunkPosition = worldManager.worldToChunkPosition(centerWorldPosition);

    for (int x = -updateDistance; x <= updateDistance; x++) {
        for (int y = -updateDistance; y <= updateDistance; y++) {
            for (int z = -updateDistance; z <= updateDistance; z++) {
                Vec3 chunkPosition = centerChunkPosition + Vec3(x, y, z);

                Vec3 chunkWorldCenter = chunkPosition * worldManager.getChunkSize() + Vec3(worldManager.getChunkSize() / 2.0f);
                float distance = length(chunkWorldCenter - centerWorldPosition);
                if (distance > updateDistance * worldManager.getChunkSize()) {
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
    frequency = 0.05f / 16;
    amplitude = 70.0f;
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

    if (height < 0) {
        if (temp > 0.33) return "ocean";
        else return "coldOcean";
    } else if (height > 100) {
        if (temp > 0.33) return "mountains";
        else return "snowyMountains";
    }

    if (temp > 0.66) {
        if (humid > 0.5) res = "rainForest";
        else res = "dessert";
    } else if (temp > 0.33) {
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
            float height = generateHeight(wp.x + x, wp.z + z);
            std::string biome = generateBiome(wp, height);
            Voxel newVoxel;

            if (biome == "ocean") {
                newVoxel.materialID = 8;
                for (int y = 0; y < chunkSize; y++) {
                    if (wp.y + y > height && wp.y + y < 0) {
                        chunk->addVoxel(wp + Vec3(x, y, z), newVoxel);  
                    }
                } 
            }
            
            for (int y = 0; y + wp.y < height && y < chunkSize; y++) {
                if (wp.y + y >= (int)height - 1) {
                    newVoxel.materialID = 5;
                } else if (wp.y + y > (int)height -4) {
                    newVoxel.materialID = 4;
                } else {
                    newVoxel.materialID = 1;
                }
                chunk->addVoxel(wp + Vec3(x, y, z), newVoxel);   
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
                if (val > 0.3) chunk->addVoxel(wp + Vec3(x, y, z), {5,0}); 
            }
        }
    }
}
