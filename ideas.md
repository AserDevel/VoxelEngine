* dynamic objects with physics and rotation and custom size voxels (could include trees, grass and other high details objects)
* lightemitting materials
* integer vector
* infinite height, heightmap stored in worldmanager, seperate chunkmanager class, no subChunks
* clouds and floating islands, different types of worldgeneration based on biome
* LOD for far away chunks to improve render distance, general improvement in the rasterization and rendering pipeline
* Reimplement shadow mapping (remove directional skylight, but still use normals for block lights)