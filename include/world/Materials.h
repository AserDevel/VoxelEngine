#pragma once

#include "utilities/standard.h"

struct Material {
    Vec3 color;
    float reflectivity;
    uint32_t shininess;
};

const Material stone = { Vec3(0.3, 0.3, 0.3), 0.0, 0 };
const Material metal = { Vec3(0.5, 0.5, 0.5), 0.5, 16 };
const Material wood = { Vec3(0.8, 0.6, 0.4), 0.0, 0 };
const Material ice = { Vec3(0.5, 0.5, 1.0), 1.0, 32 };
const Material dirt = { Vec3(0.7, 0.4, 0.1), 0.0, 0 };
const Material grass = { Vec3(0.1, 0.9, 0.1), 0.1, 2 };
const Material snow = { Vec3(1.0, 1.0, 1.0), 0.2, 2 };
const Material sand = { Vec3(1.0, 1.0, 0.0), 0.0, 0 };
const Material water = { Vec3(0.0, 0.0, 1.0), 1.0, 32 };

static Material materials[256] = {
    stone, 
    metal, 
    wood, 
    ice,
    dirt,
    grass,
    snow,
    sand,
    water,
};

