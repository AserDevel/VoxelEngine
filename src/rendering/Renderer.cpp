#include "rendering/Renderer.h"

void Renderer::render(Camera& camera) {
    chunks.clear();

    Vec3 centerChunk = worldManager.worldToChunkPosition(camera.position);
    
    for (int x = -renderDistance; x <= renderDistance; x++) {
        for (int y = -renderDistance; y <= renderDistance; y++) {
            for (int z = -renderDistance; z <= renderDistance; z++) {
                if (worldManager.chunkInCache(centerChunk + Vec3(x, y, z))) {
                    auto chunk = worldManager.getChunkAt(centerChunk + Vec3(x, y, z));

                    if (chunk->state == ChunkState::PENDING) {
                        continue;
                    }

                    if (chunk->state == ChunkState::READY) {
                        chunk->state = ChunkState::PENDING;
                        threadManager.addTask([this, chunk]() {
                            generateChunkMesh(chunk);
                        });
                        continue;
                    }

                    if (chunk->state == ChunkState::MESHED) {
                        chunk->mesh.loadToGPU();
                        chunk->state = ChunkState::LOADED;
                    }
                    
                    chunks.push_back(chunk);
                }
            }
        }
    }

    shader.use();
    
    shader.bindMatrix(camera.getMatCamera(), "matCamera");
    shader.bindVector(camera.position, "eyePos");
    shader.bindFloat(globalAmbience, "globalAmbience");
    shader.bindVector(Vec3(-0.5, -0.8, -0.5), "directionalLightDir");
    shader.bindVector(Vec3(1.0, 0.9, 0.9), "directionalLightColor");
    shader.bindMaterials(materials);

    for (auto chunk : chunks) {
        if (chunk->state != ChunkState::LOADED) continue;
        chunk->mesh.bind();
        chunk->mesh.draw();
    }
}

void Renderer::generateChunkMesh(std::shared_ptr<Chunk> chunk) {
    chunk->mesh.vertices.clear();
    chunk->mesh.indices.clear();    

    for (const auto& [index, voxel] : chunk->activeVoxels) {
        Vec3 localPosition = chunk->indexToPosition(index); 
        // For each face of the cube
        for (int i = 0; i < 6; i++) {
            Vec3 neighborPos = localPosition + cubeNormals[i];

            // the face is an edge if the neighbor is out of bounds and empty/transparent
            bool isEdge = !chunk->positionInBounds(neighborPos) && 
                        !worldManager.voxelExistsAt(chunk->worldPosition + neighborPos);

            if (chunk->positionIsTransparent(neighborPos) || isEdge) {
                // Transform face vertices to world coordinates
                for (int v = 0; v < 4; v++) {
                    Vertex vertex = cubeVertices[i][v];
                    vertex.position += chunk->worldPosition + localPosition;
                    vertex.materialID = voxel.materialID;
                    chunk->mesh.vertices.push_back(vertex);
                }
                
                // Add face indices, offset by current vertex count
                GLuint baseIndex = chunk->mesh.vertices.size() - 4;
                for (int i = 0; i < 6; i++) {
                    chunk->mesh.indices.push_back(baseIndex + cubeIndicies[i]);
                }
            }
        }
    }

    chunk->state = ChunkState::MESHED;
    chunk->isDirty = false;
}