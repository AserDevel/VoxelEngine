#include "utilities/standard.h"

class WorldManager;

class LightGenerator {
private:
    WorldManager& worldManager;

public:
    LightGenerator(WorldManager& worldManager)
        : worldManager(worldManager) {}

    uint8_t calculateLightAt(const Vec3& worldPosition);

    void removeLight(const Vec3& worldPosition, uint8_t lightLevel);

    void propagateLight(const Vec3& worldPosition, uint8_t lightLevel);
};
