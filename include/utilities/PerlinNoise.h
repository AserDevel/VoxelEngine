#pragma once

#include <numeric>
#include <random>

class PerlinNoise {
public:
    PerlinNoise();                   // Default constructor
    PerlinNoise(unsigned int seed);  // Constructor with a seed

    double noise(double x, double y) const;      // 2D Perlin Noise
    double noise(double x, double y, double z) const; // 3D Perlin Noise

private:
    std::vector<int> permutation;

    double fade(double t) const;            // Smoothing function
    double lerp(double t, double a, double b) const; // Linear interpolation
    double grad(int hash, double x, double y, double z) const; // Gradient function
    double grad(int hash, double x, double y) const; // Gradient function (2D)
};
