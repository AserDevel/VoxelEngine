#pragma once

#include "utilities/standard.h"

struct Material {
    Vec4 color;
    bool isTransparent;
};

enum MaterialID {
    ID_AIR,
    ID_STONE,
    ID_METAL,
    ID_WOOD,
    ID_ICE,
    ID_DIRT,
    ID_GRASS,
    ID_SNOW,
    ID_SAND,
    ID_WATER,
    ID_LEAVES,
};

const Material air = { Vec4(1.0, 1.0, 1.0, 0.0), true};
const Material stone = { Vec4(0.5, 0.5, 0.5, 1.0), false };
const Material metal = { Vec4(0.5, 0.5, 0.5, 1.0), false };
const Material wood = { Vec4(0.4, 0.3, 0.1, 1.0), false };
const Material ice = { Vec4(0.5, 0.5, 1.0, 1.0), false };
const Material dirt = { Vec4(0.6, 0.4, 0.2, 1.0), false };
const Material grass = { Vec4(0.2, 0.9, 0.2, 1.0), false };
const Material snow = { Vec4(1.0, 1.0, 1.0, 1.0), false };
const Material sand = { Vec4(1.0, 1.0, 0.0, 1.0), false };
const Material water = { Vec4(0.0, 0.0, 1.0, 0.5), true };
const Material leaves = { Vec4(0.2, 0.6, 0.2, 1.0), false };

static Material materials[256] = {
    air,
    stone, 
    metal, 
    wood, 
    ice,
    dirt,
    grass,
    snow,
    sand,
    water,
    leaves
};