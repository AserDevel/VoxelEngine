#pragma once

#include "utilities/standard.h"

struct Material {
    Vec4 color = Vec4(0.0);
    float specularity = 0.0;

    Material(Vec4 color = Vec4(0.0), float specularity = 0.0)
        : color(color), specularity(specularity) {}
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

const Material air = Material();
const Material stone = Material(Vec4(0.5, 0.5, 0.5, 1.0));
const Material metal = Material(Vec4(0.7, 0.7, 0.7, 1.0), 0.9);
const Material wood = Material(Vec4(0.4, 0.3, 0.1, 1.0));
const Material ice = Material(Vec4(0.5, 0.5, 1.0, 1.0));
const Material dirt = Material(Vec4(0.6, 0.4, 0.2, 1.0));
const Material grass = Material(Vec4(0.2, 0.9, 0.2, 1.0));
const Material snow = Material(Vec4(1.0, 1.0, 1.0, 1.0));
const Material sand = Material(Vec4(1.0, 1.0, 0.0, 1.0));
const Material water = Material(Vec4(0.2, 0.2, 1.0, 0.5), 1.0);
const Material leaves = Material(Vec4(0.2, 0.6, 0.2, 1.0));

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