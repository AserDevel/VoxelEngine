#ifndef VEC3_H
#define VEC3_H

#include <math.h>
#include <iostream>
#include "Utils.h"

struct Vec3 {
    union { float x = 0, r, u; };
    union { float y = 0, g, v; };
    union { float z = 0, b, w; };

    inline Vec3(float a = 0, float b = 0, float c = 0) : 
        x(a), y(b), z(c) {};
    
    inline bool operator==(const Vec3& other) const {
        const float epsilon = 1e-5f;  // Precision tolerance
        return std::abs(x - other.x) < epsilon &&
               std::abs(y - other.y) < epsilon &&
               std::abs(z - other.z) < epsilon;
    }

    inline bool operator!=(const Vec3& other) const {
        const float epsilon = 1e-5f;  // Precision tolerance
        return std::abs(x - other.x) > epsilon ||
               std::abs(y - other.y) > epsilon ||
               std::abs(z - other.z) > epsilon;
    }

    inline Vec3 operator+(const Vec3& v) const {
        Vec3 vec;
        vec.x = this->x + v.x; vec.y = this->y + v.y; vec.z = this->z + v.z;
        return vec;
    }

    inline void operator+=(const Vec3& v) {
        this->x += v.x; this->y += v.y; this->z += v.z;
    }

    inline Vec3 operator-(const Vec3& v) const {
        Vec3 vec;
        vec.x = this->x - v.x; vec.y = this->y - v.y; vec.z = this->z - v.z;
        return vec;
    }

    inline void operator-=(const Vec3& v) {
        this->x -= v.x; this->y -= v.y; this->z -= v.z;
    }

    inline Vec3 operator-() const {
        Vec3 vec;
        vec.x = -this->x; vec.y = -this->y; vec.z = -this->z;
        return vec;
    }

    inline Vec3 operator*(const Vec3& v) const {
        return { this->x * v.x, this->y * v.y, this->z * v.z };
    }

    inline Vec3 operator/(const Vec3& v) const {
        return { this->x / v.x, this->y / v.y, this->z / v.z };
    }

    inline Vec3 operator*(const float k) const {
        Vec3 vec;
        vec.x = this->x * k; vec.y = this->y * k; vec.z = this->z * k;
        return vec;
    }

    inline Vec3 operator/(const float k) const {
        Vec3 vec;
        vec.x = this->x / k; vec.y = this->y / k; vec.z = this->z / k;
        return vec;
    }

    inline void print() {
        std::cout << this->x << ", " << this->y << ", " << this->z << std::endl;
    }
};

inline float dot(const Vec3& v1, const Vec3& v2) {
    return (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z);
}

inline float length(const Vec3& v) {
    return sqrt(dot(v, v));
}

inline Vec3 normalise(const Vec3& v) {
    float l = length(v);
    return { v.x / l, v.y / l, v.z / l };
}

inline Vec3 cross(const Vec3& v1, const Vec3& v2) {
    Vec3 v;
    v.x = v1.y * v2.z - v1.z * v2.y;
    v.y = v1.z * v2.x - v1.x * v2.z;
    v.z = v1.x * v2.y - v1.y * v2.x;
    return v;
}

inline Vec3 sign(const Vec3& vec) {
    return Vec3(sign(vec.x), sign(vec.y), sign(vec.z));
}

inline Vec3 floor(const Vec3 vec) {
    return Vec3(std::floor(vec.x), std::floor(vec.y), std::floor(vec.z));
}

inline Vec3 abs(const Vec3 vec) {
    return Vec3(std::abs(vec.x), std::abs(vec.y), std::abs(vec.z));
}


#endif