#pragma once

#include "utilities/standard.h"
#include "rendering/Camera.h"
#include "physics/PhysicsEngine.h"
#include "world/WorldManager.h"

class Player {
private:
    float baseSpeed = 10.0;
    Voxel inventory[9];
    int selectedItem = 0;

    WorldManager& worldManager;

public:
    Shape shape;
    Camera camera;

    Player(const Vec3& startPosition, WorldManager& worldManager);

    void processInput();

    void update();

    void handleEvent(const SDL_Event& event);
};
