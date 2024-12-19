#ifndef VEC2_H
#define VEC2_H

#include <math.h>
#include <iostream>

struct Vec2 {
    union { float x = 0, u; };
    union { float y = 0, v; };

    inline Vec2(float x = 0, float y = 0) : 
        x(x), y(y) {}
    
    inline Vec2 operator+(const Vec2& v) const {
        Vec2 vec;
        vec.x = this->x + v.x; vec.y = this->y + v.y;
        return vec;
    }

    inline void operator+=(const Vec2& v) {
        this->x += v.x; this->y += v.y;
    }

    inline Vec2 operator-(const Vec2& v) const {
        Vec2 vec;
        vec.x = this->x - v.x; vec.y = this->y - v.y;
        return vec;
    }

    inline Vec2 operator*(const Vec2& v) const {
        return { this->x * v.x, this->y * v.y };
    }

    inline Vec2 operator/(const Vec2& v) const {
        return { this->x / v.x, this->y / v.y };
    }

    inline void operator-=(const Vec2& v) {
        this->x -= v.x; this->y -= v.y;
    }

    inline Vec2 operator-() const {
        Vec2 vec;
        vec.x = -this->x; vec.y = -this->y;
        return vec;
    }

    inline Vec2 operator*(const float k) const {
        Vec2 vec;
        vec.x = this->x * k; vec.y = this->y * k;
        return vec;
    }

    inline Vec2 operator/(const float k) const {
        Vec2 vec;
        vec.x = this->x / k; vec.y = this->y / k;
        return vec;
    }

    inline void print() {
        std::cout << this->x << ", " << this->y << std::endl;
    }
};

inline float dot(const Vec2& v1, const Vec2& v2) {
    return (v1.x * v2.x + v1.y * v2.y);
}

inline float length(const Vec2& v) {
    return sqrt(dot(v, v));
}

inline Vec2 normalise(const Vec2& v) {
    float l = length(v);
    return { v.x / l, v.y / l };
}

inline float det(const Vec2& v1, const Vec2& v2) {
    return ((v1.x * v2.y) - (v2.x * v1.y));
}


#endif