#pragma once

#include "utilities/standard.h"
#include "world/WorldManager.h"
#include "world/ThreadManager.h"
#include "Camera.h"
#include "Shader.h"

class Renderer {
private:
    int renderDistance = 4;
    float globalAmbience = 0.1f;
    WorldManager& worldManager;
    ThreadManager& threadManager;
    std::vector<std::shared_ptr<Chunk>> chunks;
    Shader shader = Shader("assets/shaders/testShader2.glsl");

public:
    Renderer(WorldManager& worldManager, ThreadManager& threadManager)
        : worldManager(worldManager), threadManager(threadManager) {}

    ~Renderer() {}

    void setRenderDistance(int chunkRadius) {
        renderDistance = chunkRadius;
    }

    void render(Camera& playerCam);

private:
    void generateChunkMesh(std::shared_ptr<Chunk> chunk);

};