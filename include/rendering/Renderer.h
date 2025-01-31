#pragma once

#include "utilities/standard.h"
#include "world/WorldManager.h"
#include "utilities/ThreadManager.h"
#include "Camera.h"
#include "Shader.h"

class Renderer {
private:
    static const int chunkSize = CHUNKSIZE;

    int frame = 0;
    int screenWidth, screenHeight;
    
    // World buffers
    GLuint voxelBuffer;
    GLuint chunkOffsetBuffer;

    // Geometry buffer
    GLuint gBuffer;
    GLuint positionTex, normalTex, colorTex;

    // Light buffers
    GLuint frameBuffers[2], globalLightTextures[2], specialLightTextures[2];
    GLuint currentBuffer = 0;

    GLuint blueNoiseTex;

    GLuint markerVAO, markerVBO;
    GLuint screenVAO, screenVBO;

    Vec3 lightDir;
    Vec3 lightColor;

    Shader monoColorShader = Shader("assets/shaders/basic2D.vert", "assets/shaders/monoColor.frag");
    Shader geometryShader = Shader("assets/shaders/basic3D.vert", "assets/shaders/geometry.frag");
    Shader globalLightShader = Shader("assets/shaders/basic3D.vert", "assets/shaders/globalLight.frag");
    Shader specialLightShader = Shader("assets/shaders/basic3D.vert", "assets/shaders/specialLight.frag");
    Shader denoiser = Shader("assets/shaders/basic3D.vert", "assets/shaders/denoiser.frag");
    Shader assembler = Shader("assets/shaders/basic3D.vert", "assets/shaders/assembler.frag");

    Camera& camera;
    WorldManager& worldManager;

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

private:
    void initGLSettings();

    void loadScreenQuad();

    void loadMarker();

    void loadWorldBuffers();

    void updateWorldBuffers();

    void loadLightBuffers();

    void loadGBuffer();

    void loadBlueNoiseTexture();
};