#include "world/Chunk.h"
#include <queue>

int SubChunk::positionToIndex(const Vec3& localPosition) const {
    int idx = localPosition.x + (localPosition.y * size) + (localPosition.z * size * size);
    if (idx < 0 || idx >= (size * size * size)) {
        std::cerr << "subchunk does not contain the given local position: "; localPosition.print();
        return -1;
    } 
    
    return idx;
}

void SubChunk::forEachVoxel(std::function<void(const Vec3&, const Voxel&)> callback) const {
    int idx = 0;
    for (int z = 0; z < size; z++) {
        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++, idx++) {
                const Vec3 position(x, y, z);
                const Voxel& voxel = voxels[idx];
                callback(position, voxel);
            }
        }
    }
}

void SubChunk::forEachVoxel(std::function<void(const Vec3&, Voxel&)> callback) {
    int idx = 0;
    for (int z = 0; z < size; z++) {
        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++, idx++) {
                const Vec3 position(x, y, z);
                Voxel& voxel = voxels[idx];
                callback(position, voxel);
            }
        }
    }
}

// voxel manipulation with bounds and dirty flag handling
bool SubChunk::addVoxel(const Vec3& localPosition, const Voxel& voxel) {
    int idx = positionToIndex(localPosition);
    if (voxels[idx].materialID != IDX_AIR) {
        std::cerr << "Attempted to replace a solid voxel" << std::endl;
        return false;
    }    
    voxels[idx] = voxel;
    return true;
}

bool SubChunk::removeVoxel(const Vec3& localPosition) {
    int idx = positionToIndex(localPosition);
    if (voxels[idx].materialID == IDX_AIR) {
        std::cerr << "Attempted to remove a non-solid voxel" << std::endl;
        return false;
    }
    voxels[idx] = {0, 0, 0};
    return true;
}

Voxel* SubChunk::getVoxelAt(const Vec3& localPosition) {
    int idx = positionToIndex(localPosition);
    return &voxels[idx];
}

// bounds checking
bool SubChunk::positionInBounds(const Vec3& localPosition) const {
    return localPosition.x >= 0 && localPosition.x < size &&
           localPosition.y >= 0 && localPosition.y < size &&
           localPosition.z >= 0 && localPosition.z < size;
}

bool SubChunk::positionIsEdge(const Vec3& localPosition) const {
    if (!positionInBounds(localPosition)) return false;
    return localPosition.x == 0 || localPosition.x == size-1 ||
           localPosition.y == 0 || localPosition.y == size-1 ||
           localPosition.z == 0 || localPosition.z == size-1;
}

bool SubChunk::positionIsSolid(const Vec3& localPosition) {
    Voxel* voxel;
    if (positionInBounds(localPosition)) {
        voxel = getVoxelAt(localPosition);
    } else {
        Vec3 worldPosition = this->worldPosition + localPosition;
        auto neighbour = parentChunk->getNeighbourAt(worldPosition);
        if (!neighbour) return false;
        voxel = neighbour->getVoxelAt(worldPosition - neighbour->worldPosition);
    }
    if (!voxel) return false;
    return voxel->materialID != IDX_AIR;
}

bool SubChunk::positionIsTransparent(const Vec3& localPosition) {
    Voxel* voxel;
    if (positionInBounds(localPosition)) {
        voxel = getVoxelAt(localPosition);
    } else {
        Vec3 worldPosition = this->worldPosition + localPosition;
        auto neighbour = parentChunk->getNeighbourAt(worldPosition);
        if (!neighbour) return true;
        voxel = neighbour->getVoxelAt(worldPosition - neighbour->worldPosition);
    }
    if (!voxel) return true;
    return materials[voxel->materialID].isTransparent;
}

uint8_t SubChunk::getLightLevelAt(const Vec3& localPosition) {
    Voxel* voxel;
    if (positionInBounds(localPosition)) {
        voxel = getVoxelAt(localPosition);
    } else {
        Vec3 worldPosition = this->worldPosition + localPosition;
        auto neighbour = parentChunk->getNeighbourAt(worldPosition);
        if (!neighbour) return 0x0F;
        voxel = neighbour->getVoxelAt(worldPosition - neighbour->worldPosition);
    }
    if (!voxel) return 0x0F;
    return voxel->lightLevel;
}

void SubChunk::updateLightSources() {
    forEachVoxel([&](const Vec3& localPosition, Voxel& voxel) {
        if (voxel.materialID == IDX_AIR) {
            int topHeight = parentChunk->getHeightAt(Vec2(localPosition.x, localPosition.z)); 
            if (this->worldPosition.y + localPosition.y >= topHeight) {
                voxel.lightLevel = 0x0F; // set skylight to maximum
            } else {
                voxel.lightLevel = 0x00; // set skylight to minimum
            }
        }
    });
}

void SubChunk::updateLight() {
    forEachVoxel([&](const Vec3& localPosition, Voxel& voxel) {
        if (voxel.materialID == IDX_AIR) {
            voxel.lightLevel = calculateSkyLightAt(localPosition);
        }
    });
}

// calculate skylight at the given position
int SubChunk::calculateSkyLightAt(const Vec3& localPosition) {
    int lightLevel = 15;
    std::queue<Vec3> positions;
    std::unordered_set<Vec3, Vec3Hash> visited;

    // Push the initial position
    positions.push(localPosition);
    visited.insert(localPosition);

    int positionCounter = positions.size(); // Start with the initial position count

    while (lightLevel > 0 && !positions.empty()) {
        const Vec3& currPos = positions.front();
        positions.pop();
        positionCounter--;

        // Check if the current position is a skylight source
        if ((getLightLevelAt(currPos) & 0x0F) == 0x0F) {
            return lightLevel;
        }

        // Enqueue neighbors
        for (int face = 0; face < 6; face++) {
            const Vec3& newPos = currPos + cubeNormals[face];

            if (!positionIsSolid(newPos)) {
                // Only process unvisited positions
                if (visited.find(newPos) == visited.end()) {
                    positions.push(newPos);
                    visited.insert(newPos);
                }
            }
        }

        // If we've processed all positions at the current level, reduce the light level
        if (positionCounter == 0) {
            lightLevel--;
            positionCounter = positions.size(); // Reset counter for the next level
        }
    }

    return 0;
}

void SubChunk::generateMesh() {
    mesh.vertices.clear();
    mesh.indices.clear();
    transparentMesh.vertices.clear();
    transparentMesh.indices.clear();
    
    forEachVoxel([&](const Vec3& localPosition, const Voxel& voxel) {
        if (voxel.materialID != IDX_AIR) {
            bool isTransparent = materials[voxel.materialID].isTransparent;
            
            // For each face of the cube
            for (int face = 0; face < 6; face++) {
                const Vec3& neighborPos = localPosition + cubeNormals[face];
                bool faceIsVisible = false;

                if (isTransparent) {
                    faceIsVisible = !positionIsSolid(neighborPos);
                } else {
                    faceIsVisible = positionIsTransparent(neighborPos);
                }
                
                if (faceIsVisible) {
                    // Transform face vertices to world coordinates
                    int a00a11 = 0, a01a10 = 0;
                    for (int vert = 0; vert < 4; vert++) {
                        Vertex vertex = cubeVertices[face][vert];
                        
                        uint8_t lightLevel = getLightLevelAt(neighborPos);
                        vertex.position += this->worldPosition + localPosition;
                        vertex.data |= lightLevel << OFFSET_LIGHTLEVEL;
                        vertex.data |= face << OFFSET_NORMAL;
                        vertex.data |= voxel.materialID << OFFSET_MATERIALID;
                        
                        if (isTransparent) {
                            vertex.data |= 3 << OFFSET_AO;
                            transparentMesh.vertices.push_back(vertex);
                            continue;
                        }
                        
                        int AOvalue = vertexAO(face, vert, localPosition);
                        vertex.data |= AOvalue << OFFSET_AO;
                        
                        if (vert < 2) {
                            a00a11 += AOvalue;
                        } else {
                            a01a10 += AOvalue;
                        }  
                        
                        mesh.vertices.push_back(vertex);
                    }
                    
                    // Add face indices, offset by current vertex count
                    GLuint baseIndex = transparentMesh.vertices.size() - 4;
                    if (isTransparent) {
                        for (int i = 0; i < 6; i++) {
                            transparentMesh.indices.push_back(baseIndex + cubeIndicies[i]);
                        }
                        continue;
                    }
                    
                    baseIndex = mesh.vertices.size() - 4;
                    if (a00a11 > a01a10) {
                        for (int i = 0; i < 6; i++) {
                            mesh.indices.push_back(baseIndex + cubeIndicies[i]);
                        }
                    } else {
                        for (int i = 0; i < 6; i++) {
                            mesh.indices.push_back(baseIndex + cubeIndiciesFlipped[i]);
                        }
                    }
                }
            }
        }     
    });
    
    isDirty = false;
}

int SubChunk::vertexAO(int i, int v, const Vec3& localVoxelPos) {
    Vec3 normal = cubeNormals[i];
    Vec3 position = sign(cubeVertices[i][v].position);

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
    if (!positionIsTransparent(localVoxelPos + side1Pos)) side1 = 1;
    if (!positionIsTransparent(localVoxelPos + side2Pos)) side2 = 1;
    if (!positionIsTransparent(localVoxelPos + cornerPos)) corner = 1;

    if (side1 && side2) return 0;
    
    return 3 - (side1 + side2 + corner);
}