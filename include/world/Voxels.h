#pragma once

#include "utilities/standard.h"

struct Voxel {
    uint8_t materialID = 0;   // Lookup in material table
    uint8_t metaData = 0;     // Custom per-voxel metadata
};

struct DynamicVoxel {
    Vec3 position;        // World position
    Quat rotation;        // Orientation
    Vec3 scale;           // Scaling
    Vec3 velocity;        // Linear velocity
    Vec3 angularVelocity; // Rotational velocity
    uint8_t materialID;   // Lookup in material table
    uint8_t metaData;     // Custom per-voxel metadata
};