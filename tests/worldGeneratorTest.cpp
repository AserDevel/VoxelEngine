#include <SDL2/SDL.h>
#include <vector>
#include <cmath>
#include <iostream>
#include <math.h>
#include "utilities/PerlinNoise.h"
#include "utilities/standard.h"

PerlinNoise perlin = PerlinNoise();

float generateHeight(float x, float z) {
    float frequency = 1.0f / 16;
    float amplitude = 4.0f;

    // general heightmap
    float height = 100.0f * perlin.noise(x * 0.002f, z * 0.002f);

    // generate more detailed terrain
    for (int i = 0; i < 3; i++) {
        height += amplitude * perlin.noise(x * frequency, z * frequency);
        frequency *= 0.5f;
        amplitude *= 2.0f;
    }

    // generate steep mountains and cliffs
    float mountainThreshold = 0.3f;
    frequency = 0.1f / 16;
    amplitude = 50.0f;
    float mountainHeight = perlin.noise(x * frequency, z * frequency);
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

struct Biome {
    Vec3 biomeColor;
    float minHeight, maxHeight;
    float minHumid, maxHumid;
    float minTemp, maxTemp;  

    Biome() {};
    Biome(Vec3 color, float minHeight, float maxHeight, float minHumid, float maxHumid, float minTemp, float maxTemp)
        : biomeColor(color), minHeight(minHeight), maxHeight(maxHeight), 
          minHumid(minHumid), maxHumid(maxHumid), minTemp(minTemp), maxTemp(maxTemp) {}
};

Vec3 generateBiome(float x, float z, float height) {
    float biomeFrequency = 0.001f;
    float temp = perlin.noise(x * biomeFrequency, z * biomeFrequency) * 0.5f + 0.5f; // [0,1]
    float humid = perlin.noise(x * biomeFrequency, z * biomeFrequency) * 0.5f + 0.5f; // [0,1]

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
SDL_Texture* GenerateMap(SDL_Renderer* renderer, int width, int height) {
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
            float fx = static_cast<float>(x) * 5;
            float fy = static_cast<float>(y) * 5;

            float height = 0.5 + 0.2 * perlin.octaveNoise(fx, fy, 5, 0.6, 0.002);

            float humid = 0.5 + 0.5 * perlin.octaveNoise(fx, fy, 3, 0.5, 0.002, Vec2(1000, 1000));

            float temp = 0.5 + 0.5 * perlin.octaveNoise(fx, fy, 3, 0.5, 0.0015, Vec2(-1000, -1000));

            float blendingThreshold = 0.01;

            Vec3 biomeColors;
            Vec3 mountainColor = Vec3(0.3, 0.3, 0.3);
            Vec3 oceanColor = Vec3(0, 0, 1);
            Vec3 rainForestColor = Vec3(0, 0.8, 0);
            Vec3 forestColor = Vec3(0, 0.6, 0);
            Vec3 taigaColor = Vec3(0, 0.4, 0);
            Vec3 dessertColor = Vec3(1, 1, 0);
            Vec3 plainsColor = Vec3(0, 1, 0);
            Vec3 snowColor = Vec3(1, 1, 1);
            
            std::string biome;
            if (height > 0.6) {
                biome = "mountains";
                biomeColors = mountainColor;
            } else if (height < 0.45) {
                biome = "ocean";
                biomeColors = oceanColor;
            } else {
                if (humid > 0.5) {
                    if (temp > 0.66) {
                        biome = "rainForest";
                        biomeColors = rainForestColor;
                    } else if (temp > 0.33) {
                        biome = "forest";
                        biomeColors = forestColor;
                    } else {
                        biome = "taiga";
                        biomeColors = taigaColor;
                    }
                } else {
                    if (temp > 0.66) {
                        biome = "dessert";
                        biomeColors = dessertColor;
                    } else if (temp > 0.33) {
                        biome = "plains";
                        biomeColors = plainsColor;
                    } else {
                        biome = "snow";
                        biomeColors = snowColor;
                    }
                }
            }

            bool isLand = biome != "mountains" && biome != "ocean";

            float blendHumidFactor = 0.5 * (humid + blendingThreshold - 0.5) / blendingThreshold;
            float blendWarmFactor = 0.5 * (temp + blendingThreshold - 0.66) / blendingThreshold;
            float blendColdFactor = 0.5 * (temp + blendingThreshold - 0.33) / blendingThreshold;

            bool blendHumid = isLand && blendHumidFactor <= 1.0 && blendHumidFactor >= 0;
            bool blendWarm = isLand && blendWarmFactor <= 1.0 && blendWarmFactor >= 0;
            bool blendCold = isLand && blendColdFactor <= 1.0 && blendColdFactor >= 0;

            if (blendWarm || blendCold || blendHumid) {
                biomeColors *= 0;
                int samples = 0;
                if (blendWarm) {
                    if (humid > 0.5) {
                        biomeColors += mix(forestColor, rainForestColor, blendWarmFactor);
                    } else {
                        biomeColors += mix(plainsColor, dessertColor, blendWarmFactor);
                    }
                    samples++;
                } else if (blendCold) {
                    if (humid > 0.5) {
                        biomeColors += mix(taigaColor, forestColor, blendColdFactor);
                    } else {
                        biomeColors += mix(snowColor, plainsColor, blendColdFactor);
                    }
                    samples++;
                }
                if (blendHumid) {
                    if (temp > 0.66) {
                        biomeColors += mix(dessertColor, rainForestColor, blendHumidFactor);
                    } else if ( temp > 0.33) {
                        biomeColors += mix(plainsColor, forestColor, blendHumidFactor);
                    } else {
                        biomeColors += mix(snowColor, taigaColor, blendHumidFactor);
                    }
                    samples++;
                }
                biomeColors /= samples;
            } 
            
            Uint8 r = static_cast<Uint8>(biomeColors.r * 255);
            Uint8 g = static_cast<Uint8>(biomeColors.g * 255);
            Uint8 b = static_cast<Uint8>(biomeColors.b * 255);

            // Set the pixel as an RGBA value (grayscale for R, G, B, A = 255)
            pixels[y * width + x] = SDL_MapRGBA(surface->format, r, g, b, 255);
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
    SDL_Texture* heightMapTexture = GenerateMap(renderer, mapWidth, mapHeight);
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
