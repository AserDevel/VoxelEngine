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

void WorldGenerator::generateChunk(std::shared_ptr<Chunk> chunk) {
    Vec3 wp = chunk->worldPosition;
    int chunkSize = worldManager.getChunkSize();
    Voxel newVoxel;
    for (double x = 0; x < chunkSize; x++) {
        for (double z = 0; z < chunkSize; z++) {
            double height = 20 + 5 * perlin.noise((wp.x + x) / chunkSize, (wp.z + z) / chunkSize);

            for (int y = 0; y + wp.y < height && y < chunkSize; y++) {
                if (wp.y + y >= (int)height - 1) {
                    newVoxel.materialID = 5;
                } else if (wp.y + y > (int)height -5) {
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
