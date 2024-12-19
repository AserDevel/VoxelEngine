#pragma once

#include "utilities/standard.h"
#include "rendering/Camera.h"

class Player {
private:
    Vec3 position;
    Quat rotation;
    Camera camera;

public:
    Player();
    ~Player() {}

    Camera& getCamera() { return camera; }

    void processInput(float deltaTime);
};
