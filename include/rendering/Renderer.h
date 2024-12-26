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
    Shader liquidShader = Shader("assets/shaders/liquidShader.glsl");
    Shader markerShader = Shader("assets/shaders/marker.glsl");
    Shader rayDebug = Shader("assets/shaders/rayDebug.glsl");

    static const int chunkSize = CHUNKSIZE;

    Camera& camera;
    WorldManager& worldManager;
    ThreadManager& threadManager;
    std::vector<std::shared_ptr<Chunk>> chunks;
    
    Shader shadowShader = Shader("assets/shaders/shadowShader.glsl");
    Shader shadowMap = Shader("assets/shaders/shadowMap.glsl");
    Shader shadowDebug = Shader("assets/shaders/shadowDebug.glsl");

    GLuint localShadowMap, localDepthMapFBO;
    GLuint globalShadowMap, globalDepthMapFBO;

    GLuint markerVAO, markerVBO;
    GLuint markerShaderProgram;

public:
    Renderer(WorldManager& worldManager, ThreadManager& threadManager, Camera& camera)
        : worldManager(worldManager), threadManager(threadManager), camera(camera) {
            loadCenterMarker();
        }

    ~Renderer() {}

    void setRenderDistance(int chunkRadius) {
        renderDistance = chunkRadius;
    }

    void render();

private:
    void generateChunkMesh(std::shared_ptr<Chunk> chunk);

    void loadCenterMarker();

    bool neighboursReady(const Vec3& chunkPosition);

    // Compute ambient occlusion for a given vertex
    int vertexAO(int i, int v, const Vec3& voxelWorldPos);

    int getSkyLightLevel(std::shared_ptr<Chunk> chunk, const Vec3& localPosition);

    void initShadowMaps();

    void generateShadowMaps();

    void debugShadowMap(GLuint shadowMap);

    void debugRay(Vec3 start, Vec3 direction);

};