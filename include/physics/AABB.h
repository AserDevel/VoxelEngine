#pragma once

#include "utilities/standard.h"

struct AABB {
    Vec3 min;
    Vec3 max;
};

bool AABBoverlapDetection(const AABB& box1, const AABB& box2);

bool AABBpointIn(const Vec3& point, const AABB& box);

bool AABBrayDetection(const Vec3& point, const Vec3& direction, const AABB& box, Vec3& collisionNormal, float& tEntry, float& tExit);


