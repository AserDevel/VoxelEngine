#pragma once

#include "utilities/standard.h"

// data layout
// 0-3 skylightlevel
// 4-7 blocklightlevel
// 8-9 AO level
// 10-12 normal
// 13-20 materialID

#define OFFSET_LIGHTLEVEL 0
#define OFFSET_AO 8
#define OFFSET_NORMAL 10
#define OFFSET_MATERIALID 13

struct Vertex {
    Vec3 position;
    GLuint data;
};

extern const Vertex cubeVertices[6][4];

extern const Vec3 cubeNormals[6];

extern const GLuint cubeIndicies[6];        // Normal cube

extern const GLuint cubeIndiciesFlipped[6]; // Flipped diagonal

class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    
    // constructor
    Mesh();
    
    // destructor
    ~Mesh() { cleanup(); }

    void bind() { glBindVertexArray(VAO); }
    
    void loadToGPU();

    void draw();

private:
    GLuint VAO; // Vertex Array Object (loading attribute pointers)
    GLuint VBO; // Vertex Buffer Object (loading vertices)
    GLuint IBO; // Index Buffer Object (loading indicies to vertices)

    void cleanup();    
};

