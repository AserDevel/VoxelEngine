#pragma once

#include "utilities/standard.h"

struct Material {
    Vec4 color;
    bool isTransparent;
};

static uint8_t IDX_AIR = 0;
static uint8_t IDX_STONE = 1; 
static uint8_t IDX_METAL = 2; 
static uint8_t IDX_WOOD = 3; 
static uint8_t IDX_ICE = 4; 
static uint8_t IDX_DIRT = 5; 
static uint8_t IDX_GRASS = 6; 
static uint8_t IDX_SNOW = 7; 
static uint8_t IDX_SAND = 8; 
static uint8_t IDX_WATER = 9; 

const Material air = {Vec4(1.0, 1.0, 1.0, 0.0), true};
const Material stone = { Vec4(0.3, 0.3, 0.3, 1.0), false };
const Material metal = { Vec4(0.5, 0.5, 0.5, 1.0), false };
const Material wood = { Vec4(0.8, 0.6, 0.4, 1.0), false };
const Material ice = { Vec4(0.5, 0.5, 1.0, 1.0), false };
const Material dirt = { Vec4(0.7, 0.4, 0.1, 1.0), false };
const Material grass = { Vec4(0.1, 0.9, 0.1, 1.0), false };
const Material snow = { Vec4(1.0, 1.0, 1.0, 1.0), false };
const Material sand = { Vec4(1.0, 1.0, 0.0, 1.0), false };
const Material water = { Vec4(0.0, 0.0, 1.0, 0.5), true };

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
};

