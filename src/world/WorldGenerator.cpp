#include "world/WorldGenerator.h"

void WorldGenerator::updateChunks(Vec3 centerChunk) {
    std::vector<std::shared_ptr<Chunk>> chunkBatch;
    for (int x = -updateDistance; x <= updateDistance; x++) {
        for (int y = -updateDistance; y <= updateDistance; y++) {
            for (int z = -updateDistance; z <= updateDistance; z++) {
                Vec3 chunkPosition = centerChunk + Vec3(x, y, z);

                if (!worldManager.chunkInCache(chunkPosition)) {
                    auto chunk = worldManager.addChunk(chunkPosition);
                    chunk->state = ChunkState::PENDING;
                    chunkBatch.push_back(chunk);
                }
            }  
        }
    }
    // add task to the queue
    threadManager.addTask([this, chunkBatch]() {
        for (auto chunk : chunkBatch) {
            generateChunk(chunk, chunk->worldPosition);
        }

        for (auto chunk : chunkBatch) {
            chunk->state = ChunkState::READY;
            for (int i = 0; i < 6; i++) {
            Vec3 neighborChunkPos = worldManager.worldToChunkPosition(chunk->worldPosition) + cubeNormals[i];
                if (worldManager.chunkInCache(neighborChunkPos)) {
                    worldManager.getChunkAt(neighborChunkPos)->state = ChunkState::READY;
                }
            }
        }
    });
}

void WorldGenerator::generateChunk(std::shared_ptr<Chunk> chunk, Vec3 worldPosition) {
    int chunkSize = worldManager.getChunkSize();
    Voxel newVoxel;
    for (double x = 0; x < chunkSize; x++) {
        for (double z = 0; z < chunkSize; z++) {
            double height = 20 + 15 * perlin.noise((worldPosition.x + x) / chunkSize, (worldPosition.z + z) / chunkSize);

            for (int y = 0; y + worldPosition.y < height && y < chunkSize; y++) {
                if (worldPosition.y + y >= (int)height - 1) {
                    newVoxel.materialID = 5;
                } else if (worldPosition.y + y > (int)height -5) {
                    newVoxel.materialID = 4;
                } else {
                    newVoxel.materialID = 1;
                }

                chunk->addVoxel(worldPosition + Vec3(x, y, z), newVoxel);
            }
        }
    }
}
