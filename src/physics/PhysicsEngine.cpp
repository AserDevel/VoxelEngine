#include "physics/PhysicsEngine.h"
#include "world/WorldManager.h"

void PhysicsEngine::addShape(Shape* shape) {
    shapes.push_back(shape);
}

void PhysicsEngine::updateShape(Shape* shape, float deltaTime) {
    Vec3 pNext = shape->position + shape->velocity * deltaTime;
    AABB boxNext;
    boxNext.min = pNext - (shape->dimensions / 2.0);
    boxNext.max = pNext + (shape->dimensions / 2.0);
        
    bool collision = false;
    Vec3 d = shape->dimensions;
    Vec3 min = pNext - (d / 2.0);
    if (worldManager.positionIsSolid(floor(boxNext.min)) || 
        worldManager.positionIsSolid(floor(Vec3(min.x + d.x, min.y, min.z))) ||
        worldManager.positionIsSolid(floor(Vec3(min.x, min.y + d.y, min.z))) ||
        worldManager.positionIsSolid(floor(Vec3(min.x, min.y, min.z + d.z))) ||
        worldManager.positionIsSolid(floor(Vec3(min.x + d.x, min.y + d.y, min.z))) ||
        worldManager.positionIsSolid(floor(min + d))) {   
        collision = true;
    }

    if (collision) {
        shape->velocity = Vec3(0.0);
        shape->acceleration = Vec3(0.0);
    } else {
        shape->position = pNext;
        shape->velocity += shape->acceleration * deltaTime;
    }
}

void PhysicsEngine::update(float deltaTime) {
    for (auto shape : shapes) {
        updateShape(shape, deltaTime);
    }
}