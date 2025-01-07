#include "utilities/standard.h"
#include "Chunk.h"

class WorldManager;

class ChunkMeshGenerator {
private:
    WorldManager& worldManager;

    int vertexAO(int i, int v, const Vec3& voxelPos);

    void generateSubChunkMesh(SubChunk* subChunk);

public:
    ChunkMeshGenerator(WorldManager& worldManager)
        : worldManager(worldManager) {}

    void generateChunkMeshes(Chunk* chunk);
};
