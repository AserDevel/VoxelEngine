#include <SDL2/SDL.h>
#include <vector>
#include <cmath>
#include <iostream>
#include "utilities/PerlinNoise.h"
#include "utilities/standard.h"

PerlinNoise perlinHeight = PerlinNoise();

float generateHeight(float x, float z) {
    float frequency = 1.0f / 16;
    float amplitude = 4.0f;

    // general heightmap
    float height = 100.0f * perlinHeight.noise(x * 0.002f, z * 0.002f);

    // generate more detailed terrain
    for (int i = 0; i < 3; i++) {
        height += amplitude * perlinHeight.noise(x * frequency, z * frequency);
        frequency *= 0.5f;
        amplitude *= 2.0f;
    }

    // generate steep mountains and cliffs
    float mountainThreshold = 0.3f;
    frequency = 0.1f / 16;
    amplitude = 50.0f;
    float mountainHeight = perlinHeight.noise(x * frequency, z * frequency);
    mountainHeight = amplitude * smoothstep(mountainThreshold - 0.2f, mountainThreshold + 0.2f, mountainHeight);

    /*
    float cliffThreshold = 0.5f;
    frequency = 1.0f / 16;
    amplitude = 10.0f;
    float cliffHeight = perlinHeight.noise(x * frequency, z * frequency);
    cliffHeight = amplitude * smoothstep(cliffThreshold - 0.05f, cliffThreshold + 0.05f, cliffHeight);
    */

    height += mountainHeight;
    
    return height;
}

PerlinNoise perlinTemp = PerlinNoise();
PerlinNoise perlinHumid = PerlinNoise();

Vec3 generateBiome(float x, float z, float height) {
    float biomeFrequency = 0.001f;
    float temp = perlinTemp.noise(x * biomeFrequency, z * biomeFrequency) * 0.5f + 0.5f; // [0,1]
    float humid = perlinHumid.noise(x * biomeFrequency, z * biomeFrequency) * 0.5f + 0.5f; // [0,1]

    Vec3 color;

    if (height <= 0) return Vec3(0, 0, 1);
    if (height > 80) return Vec3(1, 1, 1);
    if (temp > 0.66f) {
        if (humid > 0.5f) color = Vec3(0, 0.3, 0); // rain forrest
        else color = Vec3(1, 1, 0); // dessert
    } else if (temp > 0.33f) {
        if (humid > 0.5f) color = Vec3(0, 1, 0); // plain forrest
        else color = Vec3(0.5, 1, 0.5); // plains
    } else {
        if (humid > 0.5f) color = Vec3(0, 0, 0.5); // cold water
        else color = Vec3(0.5, 0.5, 1); // Ice
    }

    return color;
}

// Generate the height map as an SDL texture
SDL_Texture* GenerateHeightMap(SDL_Renderer* renderer, int width, int height) {
    // Create an SDL surface to hold pixel data
    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_RGBA32);
    if (!surface) {
        std::cerr << "Failed to create SDL surface: " << SDL_GetError() << std::endl;
        return nullptr;
    }

    // Access the pixel buffer of the surface
    Uint32* pixels = (Uint32*)surface->pixels;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // Generate height value (example noise function)
            float fx = static_cast<float>(x);
            float fy = static_cast<float>(y);

            // Map heightValue to grayscale (0-255)
            float heightValue = generateHeight(fx*10, fy*10);
            heightValue = clamp(heightValue, 0, 255);
            Uint8 grayscale = static_cast<Uint8>(heightValue);

            Vec3 biomeColors = generateBiome(fx*10, fy*10, heightValue);

            Uint8 r = static_cast<Uint8>(biomeColors.r * 255);
            Uint8 g = static_cast<Uint8>(biomeColors.g * 255);
            Uint8 b = static_cast<Uint8>(biomeColors.b * 255);

            // Set the pixel as an RGBA value (grayscale for R, G, B, A = 255)
            pixels[y * width + x] = SDL_MapRGBA(surface->format, r, g, b, 255);
            //pixels[y * width + x] = SDL_MapRGBA(surface->format, grayscale, grayscale, grayscale, 255);
        }
    }

    // Create an SDL texture from the surface
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        std::cerr << "Failed to create SDL texture: " << SDL_GetError() << std::endl;
    }

    // Save the height map as an image
    if (SDL_SaveBMP(surface, "tests/heightmap.bmp") == 0) {
        std::cout << "Height map saved as heightmap.bmp" << std::endl;
    } else {
        std::cerr << "Failed to save heightmap.bmp: " << SDL_GetError() << std::endl;
    }

    // Free the surface
    SDL_FreeSurface(surface);

    return texture;
}

int main(int argc, char* argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Create SDL window and renderer
    const int mapWidth = 512;
    const int mapHeight = 512;
    SDL_Window* window = SDL_CreateWindow("Height Map", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, mapWidth, mapHeight, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Failed to create SDL window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Failed to create SDL renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Generate the height map texture
    SDL_Texture* heightMapTexture = GenerateHeightMap(renderer, mapWidth, mapHeight);
    if (!heightMapTexture) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Main loop
    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        // Render the height map texture
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, heightMapTexture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    // Cleanup
    SDL_DestroyTexture(heightMapTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
