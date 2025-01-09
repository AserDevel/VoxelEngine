#ifndef VEC4_H
#define VEC4_H

#include <math.h>
#include <iostream>
#include "Vec3.h"

// Vec4 is treated the same as Vec4, but it has a fourth element 
// for a homogeneous value.
struct Vec4 {
    union { float r = 0, x; };
    union { float g = 0, y; };
    union { float b = 0, z; };
    union { float a = 1, w; };
    
    inline Vec4(float x = 0, float y = 0, float z = 0, float w = 1) : 
        x(x), y(y), z(z), w(w) {};
        
    
    inline bool operator==(const Vec4& other) const {
        const float epsilon = 1e-5f;  // Precision tolerance
        return std::abs(x - other.x) < epsilon &&
               std::abs(y - other.y) < epsilon &&
               std::abs(z - other.z) < epsilon;
    }

    inline bool operator!=(const Vec4& other) const {
        const float epsilon = 1e-5f;  // Precision tolerance
        return std::abs(x - other.x) > epsilon ||
               std::abs(y - other.y) > epsilon ||
               std::abs(z - other.z) > epsilon;
    }

    inline Vec4 operator-() const {
        return Vec4(-this->x, -this->y, -this->z);
    }

    inline Vec4 operator+(const Vec4& other) const {
        return Vec4(this->x + other.x, this->y + other.y, this->z + other.z);
    }

    inline void operator+=(const Vec4& other) {
        this->x += other.x; this->y += other.y; this->z += other.z;
    }

    inline Vec4 operator-(const Vec4& other) const {
        return Vec4(this->x - other.x, this->y - other.y, this->z - other.z);
    }

    inline void operator-=(const Vec4& other) {
        this->x -= other.x; this->y -= other.y; this->z -= other.z;
    }

    inline Vec4 operator*(const Vec4& other) const {
        return Vec4(this->x * other.x, this->y * other.y, this->z * other.z);
    }

    inline void operator*=(const Vec4& other) {
        this->x *= other.x; this->y *= other.y; this->z *= other.z;
    }

    inline Vec4 operator/(const Vec4& other) const {
        return Vec4(this->x / other.x, this->y / other.y, this->z / other.z);
    }

    inline void operator/=(const Vec4& other) {
        this->x /= other.x; this->y /= other.y; this->z /= other.z;
    }

    inline Vec4 operator*(const float k) const {
        return Vec4(this->x * k, this->y * k, this->z * k);
    }

    inline void operator*=(const float k) {
        this->x *= k; this->y *= k; this->z *= k;
    }

    inline Vec4 operator/(const float k) const {
        return Vec4(this->x / k, this->y / k, this->z / k);
    }

    inline void operator/=(const float k) {
        this->x /= k; this->y /= k; this->z /= k;
    }

    inline Vec3 xyz() const {
        return Vec3(this->x, this->y, this->z);
    }

    inline Vec2 xy() const {
        return Vec2(this->x, this->y);
    }

    inline Vec2 xz() const {
        return Vec2(this->x, this->z);
    }

    inline Vec3 toCartesian() const {
        return (w != 0.0f) ? Vec3(x / w, y / w, z / w) : Vec3(x, y, z); // Handle w = 0 gracefully
    }

    inline void print() {
        std::cout << this->x << ", " << this->y << ", " << this->z << ", " << this->w << std::endl;
    }
};

inline float dot(const Vec4& v1, const Vec4& v2) {
    return (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z);
}

inline float length(const Vec4& v) {
    return sqrt(dot(v, v));
}

inline Vec4 normalise(const Vec4& v) {
    float l = length(v);
    return Vec4(v.x / l, v.y / l, v.z / l, v.w);
}

inline Vec3 cross(const Vec4& v1, const Vec4& v2) {
    Vec3 v;
    v.x = v1.y * v2.z - v1.z * v2.y;
    v.y = v1.z * v2.x - v1.x * v2.z;
    v.z = v1.x * v2.y - v1.y * v2.x;
    return v;
}

inline Vec4 sign(const Vec4& vec) {
    return Vec4(sign(vec.x), sign(vec.y), sign(vec.z), sign(vec.w));
}

inline Vec4 floor(const Vec4 vec) {
    return Vec4(std::floor(vec.x), std::floor(vec.y), std::floor(vec.z), std::floor(vec.w));
}

inline Vec4 abs(const Vec4 vec) {
    return Vec4(std::abs(vec.x), std::abs(vec.y), std::abs(vec.z), std::abs(vec.w));
}

inline Vec4 mix(const Vec4& vec1, const Vec4& vec2, float t) {
    return Vec4(mix(vec1.r, vec2.r, t), mix(vec1.g, vec2.g, t), mix(vec1.b, vec2.b, t), mix(vec1.a, vec2.a, t));
}

#endif