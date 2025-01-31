#ifndef VEC3_H
#define VEC3_H

#include <math.h>
#include <iostream>
#include "Vec2.h"

struct Vec3 {
    union { float x = 0, r, u; };
    union { float y = 0, g, v; };
    union { float z = 0, b, w; };

    inline Vec3() : x(0), y(0), z(0) {}

    inline Vec3(float x, float y, float z) : 
        x(x), y(y), z(z) {};
    
    inline Vec3(float xyz) : 
        x(xyz), y(xyz), z(xyz) {};
    
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

    inline Vec3 operator-() const {
        return Vec3(-this->x, -this->y, -this->z);
    }

    inline Vec3 operator+(const Vec3& other) const {
        return Vec3(this->x + other.x, this->y + other.y, this->z + other.z);
    }

    inline void operator+=(const Vec3& other) {
        this->x += other.x; this->y += other.y; this->z += other.z;
    }

    inline Vec3 operator-(const Vec3& other) const {
        return Vec3(this->x - other.x, this->y - other.y, this->z - other.z);
    }

    inline void operator-=(const Vec3& other) {
        this->x -= other.x; this->y -= other.y; this->z -= other.z;
    }

    inline Vec3 operator*(const Vec3& other) const {
        return Vec3(this->x * other.x, this->y * other.y, this->z * other.z);
    }

    inline void operator*=(const Vec3& other) {
        this->x *= other.x; this->y *= other.y; this->z *= other.z;
    }

    inline Vec3 operator/(const Vec3& other) const {
        return Vec3(this->x / other.x, this->y / other.y, this->z / other.z);
    }

    inline void operator/=(const Vec3& other) {
        this->x /= other.x; this->y /= other.y; this->z /= other.z;
    }

    inline Vec3 operator*(const float k) const {
        return Vec3(this->x * k, this->y * k, this->z * k);
    }

    inline void operator*=(const float k) {
        this->x *= k; this->y *= k; this->z *= k;
    }

    inline Vec3 operator/(const float k) const {
        return Vec3(this->x / k, this->y / k, this->z / k);
    }

    inline void operator/=(const float k) {
        this->x /= k; this->y /= k; this->z /= k;
    }

    inline Vec2 xy() const {
        return Vec2(this->x, this->y);
    }

    inline Vec2 xz() const {
        return Vec2(this->x, this->z);
    }

    inline Vec2 yz() const {
        return Vec2(this->y, this->z);
    }

    inline void print() const {
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

inline Vec3 mix(const Vec3& vec1, const Vec3& vec2, float t) {
    return Vec3(mix(vec1.r, vec2.r, t), mix(vec1.g, vec2.g, t), mix(vec1.b, vec2.b, t));
}

// Hash function for Vec3
struct Vec3Hash {
    inline std::size_t operator()(const Vec3& v) const {
        auto hashCombine = [](std::size_t seed, std::size_t value) {
            return seed ^ (value + 0x9e3779b9 + (seed << 6) + (seed >> 2));
        };

        int xInt = static_cast<int>(v.x * 1000);  // Scale to fixed precision
        int yInt = static_cast<int>(v.y * 1000);
        int zInt = static_cast<int>(v.z * 1000);

        std::size_t hash = std::hash<int>()(xInt);
        hash = hashCombine(hash, std::hash<int>()(yInt));
        hash = hashCombine(hash, std::hash<int>()(zInt));

        return hash;
    }
};

#endif