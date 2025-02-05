#pragma once

#include "utilities/standard.h"
#include "AABB.h"

#define GRAVITY -9.816

class WorldManager;

struct Shape {
    Vec3 position;
    Vec3 velocity;
    Vec3 acceleration;

    const AABB dimensions;

    Shape(const Vec3& position, const AABB dimensions)
        : position(position), dimensions(dimensions) {}
};

class PhysicsEngine {
private:
    const WorldManager& worldManager;

    std::vector<Shape*> shapes;

    void updateShape(Shape* shape);

public:
    PhysicsEngine(const WorldManager& worldManager)
        : worldManager(worldManager) {} 

    void update();
};
