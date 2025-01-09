#include "world/WorldManager.h"

uint8_t LightGenerator::calculateLightAt(const Vec3& worldPosition) {
    uint8_t lightLevel = 0x0F;
    std::queue<Vec3> positions;
    std::unordered_set<Vec3, Vec3Hash> visited;

    // Push the initial position
    positions.push(worldPosition);
    visited.insert(worldPosition);

    int positionCounter = positions.size(); // Start with the initial position count

    while (lightLevel > 0 && !positions.empty()) {
        Vec3 currPos = positions.front();
        positions.pop();
        positionCounter--;

        // Check if the current position is a skylight source
        if ((worldManager.getLightLevelAt(currPos) & 0x0F) == 0x0F) {
            return lightLevel;
        }

        // Enqueue neighbors
        for (int face = 0; face < 6; face++) {
            const Vec3& newPos = currPos + cubeNormals[face];

            if (worldManager.positionIsTransparent(newPos)) {
                // Only process unvisited positions
                if (visited.find(newPos) == visited.end()) {
                    positions.push(newPos);
                    visited.insert(newPos);
                }
            }
        }

        // If we've processed all positions at the current level, reduce the light level
        if (positionCounter == 0) {
            lightLevel--;
            positionCounter = positions.size(); // Reset counter for the next level
        }
    }

    return 0;
}

void LightGenerator::removeLight(const Vec3& worldPosition, uint8_t lightLevel) {
    std::queue<Vec3> positions;
    std::unordered_set<Vec3, Vec3Hash> visited;

    // Push the initial position
    positions.push(worldPosition);
    visited.insert(worldPosition);

    int positionCounter = positions.size(); // Start with the initial position count

    while (lightLevel > 0 && !positions.empty()) {
        // If we've processed all positions at the current level, reduce the light level
        if (positionCounter == 0) {
            lightLevel--;
            positionCounter = positions.size(); // Reset counter for the next level
        }
        
        Vec3 currPos = positions.front();
        positions.pop();
        positionCounter--;

        Voxel* voxel = worldManager.getVoxelAt(currPos);
        if (!voxel || lightLevel < voxel->lightLevel) continue;

        uint8_t newLightLevel = calculateLightAt(currPos);
        if (newLightLevel != voxel->lightLevel) {
            voxel->lightLevel = newLightLevel;
            worldManager.markDirty(currPos);
        }

        // Enqueue neighbors
        for (int face = 0; face < 6; face++) {
            const Vec3& newPos = currPos + cubeNormals[face];

            if (worldManager.positionIsTransparent(newPos)) {
                // Only process unvisited positions
                if (visited.find(newPos) == visited.end()) {
                    positions.push(newPos);
                    visited.insert(newPos);
                }
            }
            
        }
    }
}

void LightGenerator::propagateLight(const Vec3& worldPosition, uint8_t lightLevel) {
    std::queue<Vec3> positions;
    std::unordered_set<Vec3, Vec3Hash> visited;

    // Push the initial position
    positions.push(worldPosition);
    visited.insert(worldPosition);

    int positionCounter = positions.size(); // Start with the initial position count

    while (lightLevel > 0 && !positions.empty()) {
        // If we've processed all positions at the current level, reduce the light level
        if (positionCounter == 0) {
            lightLevel--;
            positionCounter = positions.size(); // Reset counter for the next level
        }
        
        Vec3 currPos = positions.front();
        positions.pop();
        positionCounter--;

        Voxel* voxel = worldManager.getVoxelAt(currPos);
        if (!voxel || lightLevel <= voxel->lightLevel) continue;

        voxel->lightLevel = lightLevel;
        worldManager.markDirty(currPos);

        // Enqueue neighbors
        for (int face = 0; face < 6; face++) {
            const Vec3& newPos = currPos + cubeNormals[face];

            if (worldManager.positionIsTransparent(newPos)) {
                // Only process unvisited positions
                if (visited.find(newPos) == visited.end()) {
                    positions.push(newPos);
                    visited.insert(newPos);
                }
            }
        }
    }
}