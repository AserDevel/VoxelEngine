#pragma once

#include "utilities/standard.h"
#include "world/Voxels.h"
#include "world/Materials.h"

// For dynamic objects
class Object {
    std::vector<DynamicVoxel> voxels;  
    std::unordered_map<uint8_t, std::shared_ptr<Material>> materials;
    Vec3 localAABBmin;             
    Vec3 localAABBmax;
    Vec3 centerOfGravity;          // Local center of gravity
    Vec3 position;                 // Global/world position of the object
    Quat rotation;                 // Global/world rotation
    float mass;                    // Mass of the object

    bool isDirty;                  // True if the mesh needs to be regenerated

    // Physics properties
    Vec3 velocity;
};