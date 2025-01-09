#include "utilities/PerlinNoise.h"

// Default constructor initializes with a random seed
PerlinNoise::PerlinNoise() {
    permutation.resize(256);

    // Fill the permutation vector with values 0-255
    std::iota(permutation.begin(), permutation.end(), 0);

    // Shuffle the permutation vector
    std::random_device rd;
    std::default_random_engine engine(rd());
    std::shuffle(permutation.begin(), permutation.end(), engine);

    // Duplicate the permutation vector to avoid overflow
    permutation.insert(permutation.end(), permutation.begin(), permutation.end());
}

// Constructor with a fixed seed
PerlinNoise::PerlinNoise(unsigned int seed) {
    permutation.resize(256);

    std::iota(permutation.begin(), permutation.end(), 0);

    std::default_random_engine engine(seed);
    std::shuffle(permutation.begin(), permutation.end(), engine);

    permutation.insert(permutation.end(), permutation.begin(), permutation.end());
}

// Fade function (smoothstep)
double PerlinNoise::fade(double t) const {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

// Linear interpolation
double PerlinNoise::lerp(double t, double a, double b) const {
    return a + t * (b - a);
}

// Gradient function for 2D noise
double PerlinNoise::grad(int hash, double x, double y) const {
    int h = hash & 3; // Only use the last 2 bits
    double u = h < 2 ? x : y;
    double v = h < 2 ? y : x;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

// Gradient function for 3D noise
double PerlinNoise::grad(int hash, double x, double y, double z) const {
    int h = hash & 15; // Use the last 4 bits
    double u = h < 8 ? x : y;
    double v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

// 2D Perlin Noise function
double PerlinNoise::noise(double x, double y) const {
    int X = static_cast<int>(std::floor(x)) & 255; // Wrap to 0-255
    int Y = static_cast<int>(std::floor(y)) & 255;

    x -= std::floor(x); // Relative x-coordinate in unit square
    y -= std::floor(y); // Relative y-coordinate in unit square

    double u = fade(x); // Fade curves
    double v = fade(y);

    int aa = permutation[X] + Y;
    int ab = permutation[X] + Y + 1;
    int ba = permutation[X + 1] + Y;
    int bb = permutation[X + 1] + Y + 1;

    return lerp(v, lerp(u, grad(permutation[aa], x, y),
                           grad(permutation[ba], x - 1, y)),
                   lerp(u, grad(permutation[ab], x, y - 1),
                           grad(permutation[bb], x - 1, y - 1)));
}

// 3D Perlin Noise function
double PerlinNoise::noise(double x, double y, double z) const {
    int X = static_cast<int>(std::floor(x)) & 255; // Wrap to 0-255
    int Y = static_cast<int>(std::floor(y)) & 255;
    int Z = static_cast<int>(std::floor(z)) & 255;

    x -= std::floor(x); // Relative x-coordinate in unit cube
    y -= std::floor(y); // Relative y-coordinate in unit cube
    z -= std::floor(z); // Relative z-coordinate in unit cube

    double u = fade(x); // Fade curves
    double v = fade(y);
    double w = fade(z);

    int aaa = permutation[permutation[X] + Y] + Z;
    int aba = permutation[permutation[X] + Y + 1] + Z;
    int aab = permutation[permutation[X] + Y] + Z + 1;
    int abb = permutation[permutation[X] + Y + 1] + Z + 1;
    int baa = permutation[permutation[X + 1] + Y] + Z;
    int bba = permutation[permutation[X + 1] + Y + 1] + Z;
    int bab = permutation[permutation[X + 1] + Y] + Z + 1;
    int bbb = permutation[permutation[X + 1] + Y + 1] + Z + 1;

    return lerp(w, lerp(v, lerp(u, grad(permutation[aaa], x, y, z),
                                  grad(permutation[baa], x - 1, y, z)),
                          lerp(u, grad(permutation[aba], x, y - 1, z),
                                  grad(permutation[bba], x - 1, y - 1, z))),
                   lerp(v, lerp(u, grad(permutation[aab], x, y, z - 1),
                                  grad(permutation[bab], x - 1, y, z - 1)),
                          lerp(u, grad(permutation[abb], x, y - 1, z - 1),
                                  grad(permutation[bbb], x - 1, y - 1, z - 1))));
}


double PerlinNoise::octaveNoise(double x, double z, int octaves, double persistence, double scale, Vec2 offset) {
    double total = 0.0f;
    double frequency = 1.0f;
    double amplitude = 1.0f;
    double maxAmplitude = 0.0f; // Used for normalization

    for (int i = 0; i < octaves; i++) {
        // Add offset to avoid symmetry
        double nx = (x + offset.x) * frequency * scale;
        double nz = (z + offset.y) * frequency * scale;

        // Add Perlin noise with current frequency and amplitude
        total += noise(nx, nz) * amplitude;

        // Increase frequency and decrease amplitude for next octave
        frequency *= 2.0f;
        amplitude *= persistence;
        maxAmplitude += amplitude;
    }

    // Normalize result to [0, 1]
    return total / maxAmplitude;
}
