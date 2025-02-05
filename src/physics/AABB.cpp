#include "physics/AABB.h"

constexpr float epsilon = 1e-5f; // Small tolerance for floating-point errors

bool AABBoverlapDetection(const AABB& box1, const AABB& box2) {
    return (box1.min.x <= box2.max.x && box1.max.x >= box2.min.x) && // Overlap on X
           (box1.min.y <= box2.max.y && box1.max.y >= box2.min.y) && // Overlap on Y
           (box1.min.z <= box2.max.z && box1.max.z >= box2.min.z);   // Overlap on Z
}

bool AABBpointIn(const Vec3& point, const AABB& box) {
    return (point.x >= box.min.x && point.x <= box.max.x) &&
           (point.y >= box.min.y && point.y <= box.max.y) &&
           (point.z >= box.min.z && point.z <= box.max.z);
}

bool AABBpointIn2D(const Vec2& point, const AABB2D& box) {
    return (point.x >= box.min.x && point.x <= box.max.x) &&
           (point.y >= box.min.y && point.y <= box.max.y);
}

bool AABBrayDetection(const Vec3& point, const Vec3& direction, const AABB& box, Vec3& collisionNormal, float& tEntry, float& tExit) {
    float inf = std::numeric_limits<float>::infinity();
    float tMinX = -inf, tMinY = -inf, tMinZ = -inf;
    float tMaxX = inf, tMaxY = inf, tMaxZ = inf;

    if (std::abs(direction.x) > epsilon) {
        tMinX = (box.min.x - point.x) / direction.x;
        tMaxX = (box.max.x - point.x) / direction.x;
        if (direction.x < 0.0f) std::swap(tMinX, tMaxX); 
    } else if (point.x < box.min.x || point.x > box.max.x) {
        return false;
    }

    if (std::abs(direction.y) > epsilon) {
        tMinY = (box.min.y - point.y) / direction.y;
        tMaxY = (box.max.y - point.y) / direction.y;
        if (direction.y < 0.0f) std::swap(tMinY, tMaxY); 
    } else if (point.y < box.min.y || point.y > box.max.y) {
        return false;
    }

    if (std::abs(direction.z) > epsilon) {
        tMinZ = (box.min.z - point.z) / direction.z;
        tMaxZ = (box.max.z - point.z) / direction.z;
        if (direction.z < 0.0f) std::swap(tMinZ, tMaxZ); 
    } else if ( point.z < box.min.z || point.z > box.max.z) {
        return false;
    }
    
    tEntry = std::max({tMinX, tMinY, tMinZ});
    tExit = std::min({tMaxX, tMaxY, tMaxZ});

    // calculate collision normal
    if (tEntry == tMinX) collisionNormal = (direction.x < 0) ? Vec3(1, 0, 0) : Vec3(-1, 0, 0);
    else if (tEntry == tMinY) collisionNormal = (direction.y < 0) ? Vec3(0, 1, 0) : Vec3(0, -1, 0);
    else if (tEntry == tMinZ) collisionNormal = (direction.z < 0) ? Vec3(0, 0, 1) : Vec3(0, 0, -1);
    
    return (tEntry <= tExit) && (tExit >= 0.0f);
}