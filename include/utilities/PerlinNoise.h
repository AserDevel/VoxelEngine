#pragma once

#include <numeric>
#include <random>
#include "utilities/standard.h"

class PerlinNoise {
public:
    PerlinNoise();                   // Default constructor
    PerlinNoise(unsigned int seed);  // Constructor with a seed

    double noise(double x, double y) const;      // 2D Perlin Noise
    double noise(double x, double y, double z) const; // 3D Perlin Noise

    double octaveNoise(double x, double z, int octaves, double persistence, double scale, Vec2 offset = Vec2(0, 0));

private:
    std::vector<int> permutation;

    double fade(double t) const;            // Smoothing function
    double lerp(double t, double a, double b) const; // Linear interpolation
    double grad(int hash, double x, double y, double z) const; // Gradient function
    double grad(int hash, double x, double y) const; // Gradient function (2D)
};
