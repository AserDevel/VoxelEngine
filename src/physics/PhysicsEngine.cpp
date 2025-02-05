#include "physics/PhysicsEngine.h"
#include "world/WorldManager.h"

void PhysicsEngine::updateShape(Shape* shape) {
    shape->position += shape->velocity;
    shape->velocity += shape->acceleration;
}

void PhysicsEngine::update() {
    for (auto shape : shapes) {
        updateShape(shape);
    }
}