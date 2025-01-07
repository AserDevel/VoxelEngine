#pragma once

#include "utilities/standard.h"
#include "world/WorldManager.h"
#include "utilities/ThreadManager.h"
#include "Camera.h"
#include "Shader.h"

class Renderer {
private:
    float globalAmbience = 0.2f;

    Vec3 lightDir;
    Vec3 lightColor;

    Shader shader = Shader("assets/shaders/standardShader.glsl");
    Shader liquidShader = Shader("assets/shaders/liquidShader.glsl");
    Shader markerShader = Shader("assets/shaders/marker.glsl");
    Shader rayDebug = Shader("assets/shaders/rayDebug.glsl");

    static const int chunkSize = SUBCHUNK_SIZE;

    Camera& camera;
    WorldManager& worldManager;
    
    Shader shadowShader = Shader("assets/shaders/shadowShader.glsl");
    Shader shadowMap = Shader("assets/shaders/shadowMap.glsl");
    Shader shadowDebug = Shader("assets/shaders/shadowDebug.glsl");

    GLuint localShadowMap, localDepthMapFBO;
    GLuint globalShadowMap, globalDepthMapFBO;

    GLuint markerVAO, markerVBO;
    GLuint markerShaderProgram;

public:
    Renderer(WorldManager& worldManager, Camera& camera)
        : worldManager(worldManager), camera(camera) {
            loadCenterMarker();
        }

    ~Renderer() {}

    void render();

private:
    void loadCenterMarker();

    void initShadowMaps();

    void generateShadowMaps();

    void debugShadowMap(GLuint shadowMap);

    void debugRay(Vec3 start, Vec3 direction);
};