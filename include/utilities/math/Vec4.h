#ifndef VEC4_H
#define VEC4_H

#include <math.h>
#include <iostream>

// Vec4 is treated the same as Vec4, but it has a fourth element 
// for a homogeneous value.
struct Vec4 {
    union { float r = 0, x; };
    union { float g = 0, y; };
    union { float b = 0, z; };
    union { float a = 1, w; };
    
    inline Vec4(float a = 0, float b = 0, float c = 0, float d = 1) : 
        x(a), y(b), z(c), w(d) {};
        
    inline Vec4 operator+(const Vec4& v) const {
        Vec4 vec;
        vec.x = this->x + v.x; vec.y = this->y + v.y; vec.z = this->z + v.z;
        return vec;
    }

    inline void operator+=(const Vec4& v) {
        this->x += v.x; this->y += v.y; this->z += v.z;
    }

    inline Vec4 operator-(const Vec4& v) const {
        Vec4 vec;
        vec.x = this->x - v.x; vec.y = this->y - v.y; vec.z = this->z - v.z;
        return vec;
    }

    inline void operator-=(const Vec4& v) {
        this->x -= v.x; this->y -= v.y; this->z -= v.z;
    }

    inline Vec4 operator-() const {
        Vec4 vec;
        vec.x = -this->x; vec.y = -this->y; vec.z = -this->z;
        return vec;
    }

    inline Vec4 operator*(const Vec4& v) const {
        return { this->x * v.x, this->y * v.y, this->z * v.z };
    }

    inline Vec4 operator/(const Vec4& v) const {
        return { this->x / v.x, this->y / v.y, this->z / v.z };
    }

    inline Vec4 operator*(const float k) const {
        Vec4 vec;
        vec.x = this->x * k; vec.y = this->y * k; vec.z = this->z * k;
        return vec;
    }

    inline Vec4 operator/(const float k) const {
        Vec4 vec;
        vec.x = this->x / k; vec.y = this->y / k; vec.z = this->z / k;
        return vec;
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
    return { v.x / l, v.y / l, v.z / l };
}

inline Vec4 cross(const Vec4& v1, const Vec4& v2) {
    Vec4 v;
    v.x = v1.y * v2.z - v1.z * v2.y;
    v.y = v1.z * v2.x - v1.x * v2.z;
    v.z = v1.x * v2.y - v1.y * v2.x;
    return v;
}

#endif