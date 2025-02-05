#pragma once

#include "utilities/standard.h"
#include "physics/AABB.h"

class Entity {
    Vec3 position;
    Vec3 velocity;
    Vec3 front;
    const AABB dimensions;

public:
    Entity(const Vec3& position, const AABB& dimensions)
        : position(position), dimensions(dimensions) {}
};

