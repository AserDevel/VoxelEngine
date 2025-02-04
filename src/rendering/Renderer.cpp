#include "rendering/Renderer.h"
#include <unordered_set>
#include "SDL2/SDL_image.h"

void Renderer::render() {
    updateWorldBuffers();
    int worldChunkLen = worldManager.updateDistance * 2 + 1;
    Vec2 screenSize(screenWidth, screenHeight);
    Vec3 worldBasePos = worldManager.activeChunks[0]->worldPosition;
    Vec3 localCamPos = camera.position - worldBasePos;
    Mat4x4 prevViewProj = viewProj;
    viewProj = camera.getMatViewProj();
    
    // G-buffer pass
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    geometryShader.use();    
    geometryShader.bindInteger(worldChunkLen, "worldChunkLen");
    geometryShader.bindVector3(worldBasePos, "worldBasePos");
    geometryShader.bindVector2(screenSize, "screenSize");
    geometryShader.bindMatrix(inverse(viewProj), "invViewProj");
    geometryShader.bindVector3(localCamPos, "localCamPos");

    glBindVertexArray(screenVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Lighting pass
    int nextBuffer = 1 - currentBuffer; // Alternate buffers

    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffers[nextBuffer]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    lightingShader.use();
    lightingShader.bindInteger(camera.isDirty, "cameraMoved");
    lightingShader.bindInteger(worldChunkLen, "worldChunkLen");
    lightingShader.bindVector3(localCamPos, "eyePos");
    lightingShader.bindVector3(normalise(Vec3(0.4, 0.8, 0.7)), "skyLightDir");
    lightingShader.bindVector3(Vec3(1.0, 1.0, 1.0), "skyLightColor"); 
    lightingShader.bindVector2(Vec2(rand() % 256, rand() % 256), "randomOffset");
    lightingShader.bindVector2(screenSize, "screenSize");
    lightingShader.bindTexture(positionTex, "positionTex", 0);
    lightingShader.bindTexture(normalTex, "normalTex", 1);
    lightingShader.bindTexture(voxelTex, "voxelTex", 2);
    lightingShader.bindTexture(blueNoiseTex, "blueNoiseTex", 3);
    lightingShader.bindMaterials(materials);

    glBindVertexArray(screenVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Denoise the lighting
    denoiser.use();
    denoiser.bindInteger(camera.isDirty, "cameraMoved");
    denoiser.bindVector2(screenSize, "screenSize");
    denoiser.bindVector3(worldBasePos, "worldBasePos");
    denoiser.bindMatrix(prevViewProj, "prevViewProj");
    denoiser.bindTexture(lightingTextures[nextBuffer], "currFrame", 0);
    denoiser.bindTexture(lightingTextures[currentBuffer], "prevFrame", 1);
    denoiser.bindTexture(positionTex, "positionTex", 2);
    denoiser.bindTexture(normalTex, "normalTex", 3);
    denoiser.bindTexture(voxelTex, "voxelTex", 4);

    glBindVertexArray(screenVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Switch to the main buffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render to screen
    assembler.use();
    assembler.bindVector2(screenSize, "screenSize");
    assembler.bindTexture(lightingTextures[nextBuffer], "lightingTex", 0);
    assembler.bindTexture(voxelTex, "voxelTex", 1);
    assembler.bindMaterials(materials);

    glBindVertexArray(screenVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Render the center marker
    colorShader.use();
    colorShader.bindVector3(Vec3(1, 0, 0), "color"); // Red marker
    glBindVertexArray(markerVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4); // Draw the quad

    // Swap buffers
    currentBuffer = nextBuffer;

    // Update camera flag
    camera.isDirty = false;

    // Unbind
    glBindVertexArray(0);
    glUseProgram(0);
}

void Renderer::initGLSettings() {
    glEnable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    screenWidth = 600 * camera.aspectRatio;
    screenHeight = 600;

    glViewport(0, 0, screenWidth, screenHeight);
}

void Renderer::updateWorldBuffers() {
    int numChunks = worldManager.numChunks;
    int numVoxels = chunkSize * chunkSize * chunkSize;

    // update voxel data
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, voxelBuffer);
    int chunkOffsets[numChunks];
    for (int i = 0; i < numChunks; i++) {
        auto chunk = worldManager.activeChunks[i];
        if (chunk && chunk->isDirty) {
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, chunk->bufferOffset * sizeof(Voxel), numVoxels * sizeof(Voxel), chunk->voxels);
            chunk->isDirty = false;
        }
        if (!chunk || chunk->isEmpty) {
            chunkOffsets[i] = -1;
        } else {
            chunkOffsets[i] = chunk->bufferOffset;
        }
    }

    // update chunkOffsets
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, chunkOffsetBuffer);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, numChunks * sizeof(int), chunkOffsets);

    // Unbind
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

// Load world buffers based on current settings on update distance/number of chunks
void Renderer::loadWorldBuffers() {
    int numVoxels = chunkSize * chunkSize * chunkSize;
    int numChunks = worldManager.numChunks;
    int voxelDataSize = numChunks * numVoxels * sizeof(Voxel);
    
    glGenBuffers(1, &voxelBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, voxelBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, voxelDataSize, 0, GL_DYNAMIC_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, voxelBuffer);

    glGenBuffers(1, &chunkOffsetBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, chunkOffsetBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, numChunks * sizeof(int), 0, GL_DYNAMIC_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, chunkOffsetBuffer);

    // Unbind
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void Renderer::loadLightBuffers() {
    for (int i = 0; i < 2; i++) {
        glGenFramebuffers(1, &frameBuffers[i]);
        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffers[i]);

        glGenTextures(1, &lightingTextures[i]);
        glBindTexture(GL_TEXTURE_2D, lightingTextures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lightingTextures[i], 0);

        glDrawBuffers(1, &frameBuffers[i]);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cout << "Buffer not complete" << std::endl;
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::loadGBuffer() {
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

    glGenTextures(1, &positionTex);
    glBindTexture(GL_TEXTURE_2D, positionTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, positionTex, 0);

    glGenTextures(1, &normalTex);
    glBindTexture(GL_TEXTURE_2D, normalTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normalTex, 0);

    glGenTextures(1, &voxelTex);
    glBindTexture(GL_TEXTURE_2D, voxelTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, screenWidth, screenHeight, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, voxelTex, 0);

    GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "G-buffer not complete" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Setup VAO/VBO for the screen quad
void Renderer::loadScreenQuad() {
    Vec3 screenQuad[6] = {
        Vec3(1, 1, 1), Vec3(-1, 1, 1), Vec3(-1, -1, 1), 
        Vec3(1, -1, 1), Vec3(1, 1, 1), Vec3(-1, -1, 1)
    };    

    glGenVertexArrays(1, &screenVAO);
    glGenBuffers(1, &screenVBO);
    glBindVertexArray(screenVAO);
    glBindBuffer(GL_ARRAY_BUFFER, screenVBO);
    
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(Vec3), screenQuad, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3), (void*)(0));
    glEnableVertexAttribArray(0);

    // Unbind VAO and VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// Setup VAO/VBO for the marker
void Renderer::loadMarker() {
    float markerVertices[] = {
        -0.01f, -0.01f, // Bottom-left
        0.01f, -0.01f, // Bottom-right
        0.01f,  0.01f, // Top-right
        -0.01f,  0.01f  // Top-left
    };

    glGenVertexArrays(1, &markerVAO);
    glGenBuffers(1, &markerVBO);

    glBindVertexArray(markerVAO);

    glBindBuffer(GL_ARRAY_BUFFER, markerVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(markerVertices), markerVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Unbind VAO and VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Renderer::loadBlueNoiseTexture() {
	// Load image using SDL_image
    const char* filepath = "assets/textures/blueNoise256.png";
    SDL_Surface* surface = IMG_Load(filepath);
    if (!surface) {
        std::cerr << "Error loading image: " << filepath << " - " << IMG_GetError() << std::endl;
        return;
    }
    
    // Convert the surface to OpenGL texture format (RGBA)
    SDL_Surface* formattedSurface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
    if (formattedSurface->format->format != SDL_PIXELFORMAT_RGBA32) {
        std::cerr << "Error converting surface to RGBA32: " << filepath << std::endl;
    }
    SDL_FreeSurface(surface);

    // load texture to GPU
    glGenTextures(1, &blueNoiseTex);
    glBindTexture(GL_TEXTURE_2D, blueNoiseTex);

    // Set texture parameters (filtering, wrapping)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Upload the texture to OpenGL
    glTexImage2D(GL_TEXTURE_2D, 
                 0, 
                 GL_RGBA, 
                 formattedSurface->w, 
                 formattedSurface->h, 
                 0, 
                 GL_RGBA, 
                 GL_UNSIGNED_BYTE, 
                 formattedSurface->pixels);
    
    glGenerateMipmap(GL_TEXTURE_2D);

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "Error during texture upload: " << error << "\n";
    }
}

/*
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
    lineShader.use();
    glBindVertexArray(lineVAO);
    lineShader.bindMatrix(camera.getMatCamera(), "matCamera");
    lineShader.bindVector(Vec3(1, 0, 0), "lineColor");
    glDrawArrays(GL_LINES, 0, 2);

    // Cleanup
    glDeleteBuffers(1, &lineVBO);
    glDeleteVertexArrays(1, &lineVAO);

    glEnable(GL_DEPTH_TEST); // Re-enable depth testing
    glEnable(GL_CULL_FACE);
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
    //for (auto chunk : activeChunks) {
    //    if (chunk->state != ChunkState::LOADED) continue;
    //    chunk->draw();
    //}
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // do the same for the global buffer
    //lightPos = floor(camera.position - lightDir * (2 * renderDistance) * chunkSize);
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
    //for (auto chunk : activeChunks) {
    //    if (chunk->state != ChunkState::LOADED) continue;
    //    chunk->draw();
    //}
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
*/