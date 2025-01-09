#ifndef UTILS_H
#define UTILS_H

#define PI          3.14159f
#define INF_FLOAT   std::numeric_limits<float>::infinity();

inline float clamp(float target, float min, float max) {
    if (target < min) target = min;
    if (target > max) target = max;
    return target;
}

inline int clamp(int target, int min, int max) {
    if (target < min) target = min;
    if (target > max) target = max;
    return target;
}

inline float toRad(const float degrees) {
    return degrees / (180 / PI); 
}

inline float toDegrees(const float radians) {
    return radians * (180 / PI);
}

inline float sign(float value) {
    if (value > 0) return 1.0f;
    if (value < 0) return -1.0f;
    return 0.0f; // If value == 0
}

inline float smoothstep(float edge0, float edge1, float x) {
    float t = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f); // Normalize and clamp x
    return t * t * (3.0f - 2.0f * t); // Cubic interpolation
}

// higher t means less of a and more of b
inline float mix(float a, float b, float t) {
    t = clamp(t, 0.0f, 1.0f);
    return a * (1.0f - t) + b * t;
}


#endif