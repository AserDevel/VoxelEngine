#pragma once

#include "utilities/standard.h"

struct Material {
    Vec3 color;
    float density;
    float friction;
    float reflectivity;
    uint32_t shininess;
};

const Material stone = { Vec3(0.3, 0.3, 0.3), 0.7, 0.7, 0.2, 4 };
const Material metal = { Vec3(0.5, 0.5, 0.5), 0.9, 0.5, 0.7, 32 };
const Material wood = { Vec3(0.8, 0.6, 0.4), 0.5, 0.6, 0.1, 4 };
const Material ice = { Vec3(0.1, 0.1, 0.8), 0.5, 0.1, 0.9, 64 };
const Material dirt = { Vec3(0.7, 0.4, 0.1), 0.4, 0.6, 0.4, 8 };
const Material grass = { Vec3(0.1, 0.9, 0.1), 0.4, 0.5, 0.6, 16 };

static Material materials[256] = {
    stone, 
    metal, 
    wood, 
    ice,
    dirt,
    grass
};



