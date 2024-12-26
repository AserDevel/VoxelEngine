#include "rendering/Renderer.h"
#include <unordered_set>

void Renderer::render() {
    Vec3 centerChunk = worldManager.worldToChunkPosition(camera.position);
    
    for (int x = -renderDistance; x <= renderDistance; x++) {
        for (int y = -renderDistance; y <= renderDistance; y++) {
            for (int z = -renderDistance; z <= renderDistance; z++) {
                
                Vec3 chunkPosition = centerChunk + Vec3(x, y, z);            
                if (worldManager.chunkInCache(chunkPosition)) {
                    
                    Vec2 worldCenter2D = Vec2(camera.position.x, camera.position.z);
                    Vec2 chunkWorldCenter2D = Vec2(chunkPosition.x, chunkPosition.z) * chunkSize + Vec2(chunkSize / 2.0f, chunkSize / 2.0f);
                    float distance = length(chunkWorldCenter2D - worldCenter2D);
                    if (distance > renderDistance * chunkSize) {
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
                        chunk->transparentMesh.loadToGPU();
                        chunk->state = ChunkState::LOADED;
                    }
                    
                    chunks.push_back(chunk);
                }
            }
        }
    }

    lightDir = normalise(Vec3(-1, 0.0, -1));
    lightColor = Vec3(1, 1, 1);
    
    shader.use();
    shader.bindMatrix(camera.getMatCamera(), "matCamera");
    shader.bindMaterials(materials);
    
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    for (auto chunk : chunks) {
        if (chunk->state == ChunkState::LOADED) {
            chunk->mesh.bind();
            chunk->mesh.draw();
        }
    }
    
    liquidShader.use();
    liquidShader.bindMatrix(camera.getMatCamera(), "matCamera");

    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for (auto chunk : chunks) {
        if (chunk->state == ChunkState::LOADED) {
            chunk->transparentMesh.bind();
            chunk->transparentMesh.draw();
        }
    }
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    
    chunks.clear();

    // Render the center marker
    markerShader.use();
    glBindVertexArray(markerVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4); // Draw the quad
    glBindVertexArray(0);
    glUseProgram(0);
}

void Renderer::debugRay(Vec3 start, Vec3 direction) {
    Vec3 end = start + direction * 10.0f; // Extend the ray for visualization

    // Set up a line VAO
    float lineVertices[] = {
        start.x, start.y, start.z,
        end.x, end.y, end.z
    };

    GLuint lineVAO, lineVBO;

    //glDisable(GL_DEPTH_TEST); // Disable depth testing
    glDisable(GL_CULL_FACE);

    // Generate and bind the VAO
    glGenVertexArrays(1, &lineVAO);
    glBindVertexArray(lineVAO);
    
    // Generate and bind the VBO
    glGenBuffers(1, &lineVBO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    
    // Upload the line vertices to the GPU
    glBufferData(GL_ARRAY_BUFFER, sizeof(lineVertices), &lineVertices, GL_STATIC_DRAW);
    
    // Set up the vertex attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Use a simple shader for the line
    rayDebug.use();
    glBindVertexArray(lineVAO);
    rayDebug.bindMatrix(camera.getMatCamera(), "matCamera");
    rayDebug.bindVector(Vec3(1, 0, 0), "lineColor");
    glDrawArrays(GL_LINES, 0, 2);

    // Cleanup
    glDeleteBuffers(1, &lineVBO);
    glDeleteVertexArrays(1, &lineVAO);

    glEnable(GL_DEPTH_TEST); // Re-enable depth testing
    glEnable(GL_CULL_FACE);
}


// Function to render a marker in the center of the screen
void Renderer::loadCenterMarker() {
    // NDC coordinates for a small quad centered on (0, 0)
    float markerVertices[] = {
        -0.01f, -0.01f, // Bottom-left
        0.01f, -0.01f, // Bottom-right
        0.01f,  0.01f, // Top-right
        -0.01f,  0.01f  // Top-left
    };

    // Set up VAO/VBO for the marker
    glGenVertexArrays(1, &markerVAO);
    glGenBuffers(1, &markerVBO);

    glBindVertexArray(markerVAO);

    glBindBuffer(GL_ARRAY_BUFFER, markerVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(markerVertices), markerVertices, GL_STATIC_DRAW);

    // Enable vertex attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Unbind VAO and VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

bool Renderer::neighboursReady(const Vec3& chunkPosition) {
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
    chunk->transparentMesh.vertices.clear();
    chunk->transparentMesh.indices.clear();

    chunk->forEachVoxel([this, chunk](const Vec3& localPosition, const Voxel& voxel) {
        bool isTransparent = materials[voxel.materialID].isTransparent;
        bool isChunkEdge = (localPosition.x == 0 || localPosition.x == chunkSize - 1 ||
                            localPosition.y == 0 || localPosition.y == chunkSize - 1 ||
                            localPosition.z == 0 || localPosition.z == chunkSize - 1);
        
        // For each face of the cube
        const Vec3& chunkWorldPos = chunk->worldPosition;
        isChunkEdge = true;
        for (int face = 0; face < 6; face++) {
            const Vec3& neighborPos = localPosition + cubeNormals[face];

            bool faceIsVisible = false;
            if (isChunkEdge) {
                if (isTransparent) {
                    faceIsVisible = !worldManager.positionIsSolid(chunkWorldPos + neighborPos);
                } else {
                    faceIsVisible = worldManager.positionIsTransparent(chunkWorldPos + neighborPos);
                }
            } else { // Direct acces from chunk for less overhead
                if (isTransparent) {
                    faceIsVisible = !chunk->positionIsSolid(neighborPos);
                } else {
                    faceIsVisible = chunk->positionIsTransparent(neighborPos);
                }
            }
            
            if (faceIsVisible) {
                // Calculate light level
                int skyLightLevel = getSkyLightLevel(chunk, neighborPos);

                // Transform face vertices to world coordinates
                int a00a11 = 0, a01a10 = 0;
                for (int vert = 0; vert < 4; vert++) {
                    Vertex vertex = cubeVertices[face][vert];
                    
                    vertex.position += chunkWorldPos + localPosition;
                    vertex.materialID = voxel.materialID;
                    vertex.metaData = voxel.metaData;
                    vertex.metaData |= face << OFFSET_NORMALS;
                    vertex.metaData |= skyLightLevel << OFFSET_SKYLIGHT;

                    if (isTransparent) {
                        vertex.metaData |= 3 << OFFSET_AO;
                        chunk->transparentMesh.vertices.push_back(vertex);
                        continue;
                    }

                    int AOvalue = vertexAO(face, vert, chunkWorldPos + localPosition);
                    vertex.metaData |= AOvalue << OFFSET_AO;

                    if (vert < 2) {
                        a00a11 += AOvalue;
                    } else {
                        a01a10 += AOvalue;
                    }  
                    
                    chunk->mesh.vertices.push_back(vertex);
                }

                // Add face indices, offset by current vertex count
                GLuint baseIndex;
                if (isTransparent) {
                    baseIndex = chunk->transparentMesh.vertices.size() - 4;
                    for (int i = 0; i < 6; i++) {
                        chunk->transparentMesh.indices.push_back(baseIndex + cubeIndicies[i]);
                    }
                    continue;
                }
                
                bool flipDiagonal = a00a11 > a01a10;
                baseIndex = chunk->mesh.vertices.size() - 4;
                if (flipDiagonal) {
                    for (int i = 0; i < 6; i++) {
                        chunk->mesh.indices.push_back(baseIndex + cubeIndiciesFlipped[i]);
                    }
                } else {
                    for (int i = 0; i < 6; i++) {
                        chunk->mesh.indices.push_back(baseIndex + cubeIndicies[i]);
                    }
                }
            }
        }
    });

    chunk->state = ChunkState::MESHED;
    chunk->isDirty = false;
}

int Renderer::getSkyLightLevel(std::shared_ptr<Chunk> chunk, const Vec3& localPosition) {
    int lightLevel = 15;
    std::queue<Vec3> positions;
    std::unordered_set<int> visited;

    // Push the initial position
    positions.push(localPosition);
    visited.insert(chunk->positionToIndex(localPosition));

    int positionCounter = positions.size(); // Start with the initial position count

    while (lightLevel > 0 && !positions.empty()) {
        const Vec3& currPos = positions.front();
        positions.pop();
        positionCounter--;

        // Check height map once per position
        int heightAtColumn = chunk->heightMap[(int)currPos.x][(int)currPos.z];
        if (heightAtColumn == (int)(currPos.y + chunk->worldPosition.y)) {
            return lightLevel;
        }

        // Enqueue neighbors
        for (int face = 0; face < 6; face++) {
            const Vec3& newPos = currPos + cubeNormals[face];

            if (chunk->positionInBounds(newPos) && !chunk->positionIsSolid(newPos)) {
                // Only process unvisited positions
                int idx = chunk->positionToIndex(newPos);
                if (visited.find(idx) == visited.end()) {
                    positions.push(newPos);
                    visited.insert(idx);
                }
            }
        }

        // If we've processed all positions at the current level, reduce the light level
        if (positionCounter == 0) {
            lightLevel--;
            positionCounter = positions.size(); // Reset counter for the next level
        }
    }

    return lightLevel;
}


int Renderer::vertexAO(int i, int v, const Vec3& voxelWorldPos) {
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
    if (!worldManager.positionIsTransparent(voxelWorldPos + side1Pos)) side1 = 1;
    if (!worldManager.positionIsTransparent(voxelWorldPos + side2Pos)) side2 = 1;
    if (!worldManager.positionIsTransparent(voxelWorldPos + cornerPos)) corner = 1;

    if (side1 && side2) return 0;
    
    return 3 - (side1 + side2 + corner);
}

void Renderer::generateShadowMaps() {
    // light setup
    Vec3 lightUp = Vec3(0, 1, 0);
    Vec3 lightTarget = floor(camera.position);
    Vec3 lightPos = floor(camera.position - lightDir * 4 * chunkSize);
    
    // Set the light’s view and projection matrices
    float shadowRange = 0.5;
    Mat4x4 localProjection = MatrixOrtho(-shadowRange / 2.0f, shadowRange / 2.0f, -shadowRange / 2.0f, shadowRange / 2.0f, 0.01f, 3.0f);
    Mat4x4 localView = MatrixLookAt(lightPos, lightTarget, lightUp);
    Mat4x4 localSpaceMatrix = localProjection * localView;

    // Render the scene from the light’s point of view
    glViewport(0, 0, 4096, 4096);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.1, 4.0);
    glBindFramebuffer(GL_FRAMEBUFFER, localDepthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    shadowShader.use();
    shadowShader.bindMatrix(localSpaceMatrix, "lightSpaceMatrix");

    // Generate shadow map on chunks
    for (auto chunk : chunks) {
        if (chunk->state != ChunkState::LOADED) continue;
        chunk->mesh.bind();
        chunk->mesh.draw();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // do the same for the global buffer
    lightPos = floor(camera.position - lightDir * (2 * renderDistance) * chunkSize);
    shadowRange = 2.0f;    
    Mat4x4 globalProjection = MatrixOrtho(-shadowRange / 2.0f, shadowRange / 2.0f, -shadowRange / 2.0f, shadowRange / 2.0f, 0.01f, 4.0f);
    Mat4x4 globalView = MatrixLookAt(lightPos, lightTarget, lightUp);
    Mat4x4 globalSpaceMatrix = globalProjection * globalView;

    // Render the scene from the light’s point of view
    glBindFramebuffer(GL_FRAMEBUFFER, globalDepthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    shadowShader.use();
    shadowShader.bindMatrix(globalSpaceMatrix, "lightSpaceMatrix");

    // Generate shadow map on chunks
    for (auto chunk : chunks) {
        if (chunk->state != ChunkState::LOADED) continue;
        chunk->mesh.bind();
        chunk->mesh.draw();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    glDisable(GL_POLYGON_OFFSET_FILL);
    glViewport(0, 0, 600 * (16.0f/9.0f), 600);
}

void Renderer::initShadowMaps() {
    // Generate local shadowmap
    glGenTextures(1, &localShadowMap);
    glBindTexture(GL_TEXTURE_2D, localShadowMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 4096, 4096, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    float clampColor[] = { 1, 1, 1, 1 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, clampColor);

    // Create the framebuffer for the shadow map
    glGenFramebuffers(1, &localDepthMapFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, localDepthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, localShadowMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "error generating localShadowMap. Status: " << status << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Generate global shadowmap
    glGenTextures(1, &globalShadowMap);
    glBindTexture(GL_TEXTURE_2D, globalShadowMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 4096, 4096, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, clampColor);

    // Create the framebuffer for the shadow map
    glGenFramebuffers(1, &globalDepthMapFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, globalDepthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, globalShadowMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "error generating globalShadowMap. Status: " << status << std::endl;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Render the shadow map to screen
void Renderer::debugShadowMap(GLuint shadowMap) {    
        // Quad vertices for a fullscreen quad
    GLfloat quadVertices[] = {
        // Positions     // Texture coordinates
        -1.0f,  1.0f,   0.0f, 1.0f,
        -1.0f, -1.0f,   0.0f, 0.0f,
         1.0f, -1.0f,   1.0f, 0.0f,
         1.0f,  1.0f,   1.0f, 1.0f
    };

    GLuint quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // Bind the framebuffer that contains the shadow map (the shadow map is bound in generateShadowMap)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);  // 0 is the default framebuffer for rendering to screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use the shader program that renders the shadow map
    shadowDebug.use();
    shadowDebug.bindTexture(shadowMap, "shadowMap", 0);

    // Draw the fullscreen quad
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    
    // Unbind the VAO
    glBindVertexArray(0);
}
