#include "rendering/Renderer.h"

void Renderer::render(Camera& camera) {
    Vec3 centerChunk = worldManager.worldToChunkPosition(camera.position);
    
    for (int x = -renderDistance; x <= renderDistance; x++) {
        for (int y = -renderDistance; y <= renderDistance; y++) {
            for (int z = -renderDistance; z <= renderDistance; z++) {
                
                Vec3 chunkPosition = centerChunk + Vec3(x, y, z);            
                if (worldManager.chunkInCache(chunkPosition)) {
                    
                    Vec3 chunkWorldCenter = chunkPosition * worldManager.getChunkSize() + Vec3(worldManager.getChunkSize() / 2.0f);
                    float distance = length(chunkWorldCenter - camera.position);
                    if (distance > renderDistance * worldManager.getChunkSize()) {
                        continue;
                    } 

                    auto chunk = worldManager.getChunkAt(chunkPosition);
                    if (chunk->state == ChunkState::PENDING || !neighboursReady(chunkPosition)) {
                        continue;
                    }

                    // Mesh all chunks that are either dirty or newly generated and ready for meshing
                    if (chunk->state == ChunkState::READY) {
                        chunk->state = ChunkState::PENDING;
                        threadManager.addTask([this, chunk]() {
                            generateChunkMesh(chunk);
                        });
                        continue;
                    }

                    if (chunk->isDirty) {
                        generateChunkMesh(chunk);
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

    chunks.clear();
}

bool Renderer::neighboursReady(Vec3 chunkPosition) {
    int dependencyCount = 6;
    for (int i = 0; i < 6; i++) {
        Vec3 neighborChunkPos = chunkPosition + cubeNormals[i];
        if (worldManager.chunkInCache(chunkPosition)) {
            if (worldManager.getChunkAt(chunkPosition)->state != ChunkState::PENDING) {
                dependencyCount--;
            }
        }
    }
    return dependencyCount == 0;
}

void Renderer::generateChunkMesh(std::shared_ptr<Chunk> chunk) {
    chunk->mesh.vertices.clear();
    chunk->mesh.indices.clear();    

    int chunkSize = worldManager.getChunkSize();
    for (const auto& [index, voxel] : chunk->activeVoxels) {
        Vec3 localPosition = chunk->indexToPosition(index); 
        bool isEdge = (localPosition.x == 0 || localPosition.x == chunkSize - 1 ||
                    localPosition.y == 0 || localPosition.y == chunkSize - 1 ||
                    localPosition.z == 0 || localPosition.z == chunkSize - 1);
        // For each face of the cube
        for (int i = 0; i < 6; i++) {
            Vec3 neighborPos = localPosition + cubeNormals[i];

            bool isVisible = false;
            if (isEdge) {
                isVisible = !worldManager.voxelExistsAt(chunk->worldPosition + neighborPos);
            } else {
                isVisible = chunk->positionIsTransparent(neighborPos);
            }
                    
            if (isVisible) {
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