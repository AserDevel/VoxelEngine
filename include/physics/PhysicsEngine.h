#pragma once

#include "utilities/standard.h"
#include "AABB.h"

#define GRAVITY -9.816

class WorldManager;

struct Shape {
    Vec3 position;
    Vec3 dimensions;
    Vec3 velocity;
    Vec3 acceleration;

    Shape() = default;
    Shape(const Vec3& position, const Vec3& dimensions)
        : position(position), dimensions(dimensions) {}
};

class PhysicsEngine {
private:
    WorldManager& worldManager;

    std::vector<Shape*> shapes;

    void updateShape(Shape* shape, float deltaTime);

public:
    PhysicsEngine(WorldManager& worldManager)
        : worldManager(worldManager) {} 

    void update(float deltaTime);

    void addShape(Shape* shape);

    void removeShape(Shape* shape);
};
