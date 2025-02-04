#pragma once

#include "Materials.h"

// voxel data bits layout:
// 0-7 materialID;
// ...

struct Voxel {
    uint32_t data = 0;

    inline Voxel(uint32_t data = 0) : data(data) {}

    inline bool operator==(const Voxel& other) const {
        return data == other.data;
    }

    inline bool operator!=(const Voxel& other) const {
        return data != other.data;
    }

    inline Voxel& operator=(const Voxel& other) {
        data = other.data;
        return *this;
    }

    inline uint8_t getMatID() const {
        return (data & 0xFF);
    }

    inline void setMatID(const uint8_t materialID) {
        data |= materialID;
    } 

    inline bool isSolid() {
        return getMatID() != 0;
    }

    inline bool isTransparent() {
        return materials[getMatID()].color.a != 1.0;
    }
};


