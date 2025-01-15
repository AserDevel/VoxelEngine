#include "utilities/standard.h"
#include "Chunk.h"

class WorldManager;

class ChunkMeshGenerator {
private:
    WorldManager& worldManager;

    std::unordered_map<Vec3, Mesh, Vec3Hash> meshes;

    int vertexAO(int n, int v, const Vec3& voxelPos);

    uint8_t vertexLightLevel(int n, int v, const Vec3& voxelPos);

    void generateSubChunkMesh(SubChunk* subChunk);

    bool isOpaque(SubChunk* subChunk);

    void generateOutline(SubChunk* subChunk);

public:
    ChunkMeshGenerator(WorldManager& worldManager)
        : worldManager(worldManager) {}

    void generateChunkMeshes(Chunk* chunk);
};
