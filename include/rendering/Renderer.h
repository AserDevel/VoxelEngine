#pragma once

#include "utilities/standard.h"
#include "world/WorldManager.h"
#include "utilities/ThreadManager.h"
#include "Camera.h"
#include "Shader.h"

class Renderer {
private:
    int renderDistance = 4;
    float globalAmbience = 0.2f;

    Vec3 lightDir;
    Vec3 lightColor;

    Shader shader = Shader("assets/shaders/standardShader.glsl");

    int chunkSize = 16;

    Camera& camera;
    WorldManager& worldManager;
    ThreadManager& threadManager;
    std::vector<std::shared_ptr<Chunk>> chunks;
    
    Shader shadowShader = Shader("assets/shaders/shadowShader.glsl");
    Shader shadowMap = Shader("assets/shaders/shadowMap.glsl");
    Shader shadowDebug = Shader("assets/shaders/shadowDebug.glsl");
    GLuint localShadowMap, localDepthMapFBO;
    GLuint globalShadowMap, globalDepthMapFBO;

public:
    Renderer(WorldManager& worldManager, ThreadManager& threadManager, Camera& camera)
        : worldManager(worldManager), threadManager(threadManager), camera(camera) {
            chunkSize = worldManager.getChunkSize();
        }

    ~Renderer() {}

    void setRenderDistance(int chunkRadius) {
        renderDistance = chunkRadius;
    }

    void render();

private:
    void generateChunkMesh(std::shared_ptr<Chunk> chunk);

    bool neighboursReady(const Vec3& chunkPosition);

    // Compute ambient occlusion for a given vertex
    int vertexAO(int i, int v, const Vec3& voxelWorldPos);

    void initShadowMaps();

    void generateShadowMaps();

    void debugShadowMap(GLuint shadowMap);

};