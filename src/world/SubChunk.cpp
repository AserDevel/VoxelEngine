#include "world/Chunk.h"

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
                if (voxel.materialID == IDX_AIR) continue;
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
                if (voxel.materialID == IDX_AIR) continue;
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

bool SubChunk::positionIsSolid(const Vec3& localPosition) const {
    if (positionInBounds(localPosition)) {
        return voxels[positionToIndex(localPosition)].materialID != IDX_AIR;
    } else {
        return parentChunk->positionIsSolid(this->worldPosition + localPosition);
    }
}

bool SubChunk::positionIsTransparent(const Vec3& localPosition) const {
    if (positionInBounds(localPosition)) {
        return materials[voxels[positionToIndex(localPosition)].materialID].isTransparent;
    } else {
        return parentChunk->positionIsTransparent(this->worldPosition + localPosition);
    }
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

void SubChunk::generateMesh() {
    mesh.vertices.clear();
    mesh.indices.clear();
    transparentMesh.vertices.clear();
    transparentMesh.indices.clear();
    
    forEachVoxel([&](const Vec3& localPosition, const Voxel& voxel) {
        
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
                    
                    vertex.position += this->worldPosition + localPosition;
                    vertex.data |= (int)voxel.lightLevel << OFFSET_LIGHTLEVEL;
                    vertex.data |= face << OFFSET_NORMAL;
                    vertex.data |= (int)voxel.materialID << OFFSET_MATERIALID;
                    
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