#pragma once

#include "utilities/standard.h"

struct Voxel {
    uint8_t materialID = 0;   // Lookup in material table
    uint16_t metaData = 0;     // Custom per-voxel metadata
    uint8_t lightLevel = 0;
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
// Structure from LSB to MSB: 
// bit 0-2 normal
// bit 3-4 Ambient occlusion
// bit 5-8 blocklight level
// bit 9-12 skylight level
// bit 13-15 free

#define OFFSET_NORMALS      0
#define OFFSET_AO           3
#define OFFSET_BLOCKLIGHT   5
#define OFFSET_SKYLIGHT     9

#define RIGHT_BIT   0
#define LEFT_BIT    1
#define UP_BIT      2
#define DOWN_BIT    3
#define BACK_BIT    4
#define FRONT_BIT   5

#define AO0         0 << 3
#define AO1         1 << 3
#define AO2         2 << 3
#define AO3         3 << 3

#define BLOCKLIGHT0_BIT   0  << 5
#define BLOCKLIGHT1_BIT   1  << 5
#define BLOCKLIGHT2_BIT   2  << 5
#define BLOCKLIGHT3_BIT   3  << 5
#define BLOCKLIGHT4_BIT   4  << 5
#define BLOCKLIGHT5_BIT   5  << 5
#define BLOCKLIGHT6_BIT   6  << 5
#define BLOCKLIGHT7_BIT   7  << 5
#define BLOCKLIGHT8_BIT   8  << 5
#define BLOCKLIGHT9_BIT   9  << 5
#define BLOCKLIGHT10_BIT  10 << 5
#define BLOCKLIGHT11_BIT  11 << 5
#define BLOCKLIGHT12_BIT  12 << 5
#define BLOCKLIGHT13_BIT  13 << 5
#define BLOCKLIGHT14_BIT  14 << 5
#define BLOCKLIGHT15_BIT  15 << 5

#define SKYLIGHT0_BIT   0  << 9
#define SKYLIGHT1_BIT   1  << 9
#define SKYLIGHT2_BIT   2  << 9
#define SKYLIGHT3_BIT   3  << 9
#define SKYLIGHT4_BIT   4  << 9
#define SKYLIGHT5_BIT   5  << 9
#define SKYLIGHT6_BIT   6  << 9
#define SKYLIGHT7_BIT   7  << 9
#define SKYLIGHT8_BIT   8  << 9
#define SKYLIGHT9_BIT   9  << 9
#define SKYLIGHT10_BIT  10 << 9
#define SKYLIGHT11_BIT  11 << 9
#define SKYLIGHT12_BIT  12 << 9
#define SKYLIGHT13_BIT  13 << 9
#define SKYLIGHT14_BIT  14 << 9
#define SKYLIGHT15_BIT  15 << 9
