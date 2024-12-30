#pragma once

#include "utilities/standard.h"

struct Voxel {
    uint8_t materialID = 0;    // Lookup in material table
    uint8_t metaData = 0;      // Custom per-voxel metadata
    uint8_t lightLevel = 0;    // 0-3 skylight 4-7 blocklight
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


// Meta data

#define OFFSET_BLOCKLIGHT   4
#define OFFSET_SKYLIGHT     0
