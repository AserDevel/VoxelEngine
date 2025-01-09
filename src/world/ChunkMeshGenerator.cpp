#include "world/WorldManager.h"

void ChunkMeshGenerator::generateChunkMeshes(Chunk* chunk) {
    for (auto& [height, subChunk] : chunk->subChunks) {
        if (subChunk->isDirty) {
            generateSubChunkMesh(subChunk.get());
        }
    }
    chunk->isDirty = false;
    chunk->state = ChunkState::MESHED;
}

void ChunkMeshGenerator::generateSubChunkMesh(SubChunk* subChunk) {
    subChunk->mesh.vertices.clear();
    subChunk->mesh.indices.clear();
    subChunk->transparentMesh.vertices.clear();
    subChunk->transparentMesh.indices.clear();
    
    subChunk->forEachVoxel([&](const Vec3& localPosition, const Voxel& voxel) {
        if (voxel.materialID != IDX_AIR) {
            Vec3 worldPosition = subChunk->worldPosition + localPosition;
            bool isTransparent = materials[voxel.materialID].isTransparent;
            
            // For each face of the cube
            for (int face = 0; face < 6; face++) {
                const Vec3& neighborPos = worldPosition + cubeNormals[face];
                bool faceIsVisible = false;

                if (isTransparent) {
                    faceIsVisible = !worldManager.positionIsSolid(neighborPos);
                    if (voxel.materialID == IDX_WATER && face == 4 &&
                        worldManager.getVoxelAt(neighborPos)->materialID != IDX_WATER ) {
                        faceIsVisible = true;
                    } 
                } else {
                    faceIsVisible = worldManager.positionIsTransparent(neighborPos);
                }
                
                if (faceIsVisible) {
                    // Transform face vertices to world coordinates
                    int a00a11 = 0, a01a10 = 0;
                    for (int vert = 0; vert < 4; vert++) {
                        Vertex vertex = cubeVertices[face][vert];
                        
                        uint8_t lightLevel = vertexLightLevel(face, vert, worldPosition);
                        vertex.position += worldPosition;
                        vertex.data |= lightLevel << OFFSET_LIGHTLEVEL;
                        vertex.data |= face << OFFSET_NORMAL;
                        vertex.data |= voxel.materialID << OFFSET_MATERIALID;
                        
                        if (isTransparent) {
                            vertex.data |= 3 << OFFSET_AO;
                            vertex.position.y -= 0.125;
                            subChunk->transparentMesh.vertices.push_back(vertex);
                            continue;
                        }
                        
                        int AOvalue = vertexAO(face, vert, worldPosition);
                        vertex.data |= AOvalue << OFFSET_AO;
                        
                        if (vert < 2) {
                            a00a11 += AOvalue;
                        } else {
                            a01a10 += AOvalue;
                        }  
                        
                        subChunk->mesh.vertices.push_back(vertex);
                    }
                    
                    // Add face indices, offset by current vertex count
                    GLuint baseIndex = subChunk->transparentMesh.vertices.size() - 4;
                    if (isTransparent) {
                        for (int i = 0; i < 6; i++) {
                            subChunk->transparentMesh.indices.push_back(baseIndex + cubeIndicies[i]);
                        }
                        continue;
                    }
                    
                    baseIndex = subChunk->mesh.vertices.size() - 4;
                    if (a00a11 > a01a10) {
                        for (int i = 0; i < 6; i++) {
                            subChunk->mesh.indices.push_back(baseIndex + cubeIndicies[i]);
                        }
                    } else {
                        for (int i = 0; i < 6; i++) {
                            subChunk->mesh.indices.push_back(baseIndex + cubeIndiciesFlipped[i]);
                        }
                    }
                }
            }
        }     
    });
    
    subChunk->isDirty = false;
}

int ChunkMeshGenerator::vertexAO(int n, int v, const Vec3& voxelPos) {
    Vec3 normal = cubeNormals[n];
    Vec3 position = sign(cubeVertices[n][v].position);

    Vec3 side1Pos, side2Pos, cornerPos;

    if (normal.x != 0) {
        side1Pos = Vec3(normal.x, position.y, 0);
        side2Pos = Vec3(normal.x, 0, position.z);
        cornerPos = Vec3(normal.x, position.y, position.z); 
    } else if (normal.y != 0) {
        side1Pos = Vec3(position.x, normal.y, 0);
        side2Pos = Vec3(0, normal.y, position.z);
        cornerPos = Vec3(position.x, normal.y, position.z);
    } else {
        side1Pos = Vec3(position.x, 0, normal.z);
        side2Pos = Vec3(0, position.y, normal.z);
        cornerPos = Vec3(position.x, position.y, normal.z);
    }

    int side1 = 0, side2 = 0, corner = 0;
    if (!worldManager.positionIsTransparent(voxelPos + side1Pos)) side1 = 1;
    if (!worldManager.positionIsTransparent(voxelPos + side2Pos)) side2 = 1;
    if (!worldManager.positionIsTransparent(voxelPos + cornerPos)) corner = 1;

    if (side1 && side2) return 0;
    
    return 3 - (side1 + side2 + corner);
}

uint8_t ChunkMeshGenerator::vertexLightLevel(int n, int v, const Vec3& voxelPos) {
    Vec3 normal = cubeNormals[n];
    Vec3 position = sign(cubeVertices[n][v].position);

    Vec3 adjacents[4];
    if (normal.x != 0) {
        adjacents[0] = Vec3(normal.x, 0, 0);
        adjacents[1] = Vec3(normal.x, position.y, 0);
        adjacents[2] = Vec3(normal.x, 0, position.z);
        adjacents[3] = Vec3(normal.x, position.y, position.z); 
    } else if (normal.y != 0) {
        adjacents[0] = Vec3(0, normal.y, 0);
        adjacents[1] = Vec3(position.x, normal.y, 0);
        adjacents[2] = Vec3(0, normal.y, position.z);
        adjacents[3] = Vec3(position.x, normal.y, position.z);
    } else {
        adjacents[0] = Vec3(0, 0, normal.z);
        adjacents[1] = Vec3(position.x, 0, normal.z);
        adjacents[2] = Vec3(0, position.y, normal.z);
        adjacents[3] = Vec3(position.x, position.y, normal.z);
    }

    uint8_t samples = 0;
    uint8_t skyLight = 0;
    uint8_t blockLight = 0;
    for (int n = 0; n < 4; n++) {
        Vec3 pos = voxelPos + adjacents[n];
        if (worldManager.positionIsTransparent(pos)) {
            samples++;
            uint8_t lightLevel = worldManager.getLightLevelAt(pos);
            skyLight += lightLevel & 0x0F;
            blockLight += (lightLevel >> 4) & 0x0F;
        }
    }

    if (samples == 0) return 0;
    skyLight /= samples;
    blockLight /= samples;

    return skyLight + (blockLight << 4);
}

