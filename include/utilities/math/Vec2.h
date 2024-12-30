#ifndef VEC2_H
#define VEC2_H

#include <math.h>
#include <iostream>

struct Vec2 {
    union { float x = 0, u; };
    union { float y = 0, v, z; };

    inline Vec2(float x = 0, float y = 0) : 
        x(x), y(y) {}

    inline bool operator==(const Vec2& other) const {
        const float epsilon = 1e-5f;  // Precision tolerance
        return std::abs(x - other.x) < epsilon &&
               std::abs(y - other.y) < epsilon &&
               std::abs(z - other.z) < epsilon;
    }

    inline bool operator!=(const Vec2& other) const {
        const float epsilon = 1e-5f;  // Precision tolerance
        return std::abs(x - other.x) > epsilon ||
               std::abs(y - other.y) > epsilon ||
               std::abs(z - other.z) > epsilon;
    }
    
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

    inline void print() const {
        std::cout << this->x << ", " << this->y << std::endl;
    }

    inline void print() {
        std::cout << this->x << ", " << this->y << std::endl;
    }
};

// Hash function for Vec2
struct Vec2Hash {
    std::size_t operator()(const Vec2& v) const {
        auto hashCombine = [](std::size_t seed, std::size_t value) {
            return seed ^ (value + 0x9e3779b9 + (seed << 6) + (seed >> 2));
        };

        int xInt = static_cast<int>(v.x * 1000);  // Scale to fixed precision
        int yInt = static_cast<int>(v.y * 1000);

        std::size_t hash = std::hash<int>()(xInt);
        hash = hashCombine(hash, std::hash<int>()(yInt));

        return hash;
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