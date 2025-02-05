#pragma once

#include "utilities/standard.h"
#include "world/WorldManager.h"
#include "utilities/ThreadManager.h"
#include "Camera.h"
#include "Shader.h"

class Renderer {
private:
    int screenWidth, screenHeight;

    Mat4x4 viewProj;
    
    // World buffers
    GLuint voxelBuffer;
    GLuint chunkOffsetBuffer;

    // Geometry buffer
    GLuint gBuffer;
    GLuint positionTex, normalTex, voxelTex;

    // Light buffers
    GLuint frameBuffers[2], lightingTextures[2];
    GLuint currentBuffer = 0;

    GLuint blueNoiseTex;

    GLuint markerVAO, markerVBO;
    GLuint screenVAO, screenVBO;

    Vec3 lightDir;
    Vec3 lightColor;

    Shader colorShader = Shader("assets/shaders/basic2D.vert", "assets/shaders/color.frag");
    Shader geometryShader = Shader("assets/shaders/basic3D.vert", "assets/shaders/geometry.frag");
    Shader lightingShader = Shader("assets/shaders/basic3D.vert", "assets/shaders/lighting.frag");
    Shader denoiser = Shader("assets/shaders/basic3D.vert", "assets/shaders/denoiser.frag");
    Shader assembler = Shader("assets/shaders/basic3D.vert", "assets/shaders/assembler.frag");

    Camera& camera;
    WorldManager& worldManager;

    void initGLSettings();

    void loadScreenQuad();

    void loadMarker();

    void loadWorldBuffers();

    void updateWorldBuffers();

    void loadLightBuffers();

    void loadGBuffer();

    void loadBlueNoiseTexture();

public:
    Renderer(WorldManager& worldManager, Camera& camera)
        : worldManager(worldManager), camera(camera) {
        initGLSettings();
        loadScreenQuad();
        loadMarker();
        loadWorldBuffers();
        loadLightBuffers();
        loadGBuffer();
        loadBlueNoiseTexture();
    }

    ~Renderer() {}

    void render();
};