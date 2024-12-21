#pragma once

#include "utilities/standard.h"

struct Vertex {
    Vec3 position;
    Vec3 normal;
    GLubyte materialID;
    GLubyte AOvalue;
};

extern const Vertex cubeVertices[6][4];

extern const Vec3 cubeNormals[6];

extern const GLuint cubeIndicies[6];        // Normal cube

extern const GLuint cubeIndiciesFlipped[6]; // Flipped diagonal

class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    
    Mesh();
    ~Mesh() { cleanup(); }

    void bind() { glBindVertexArray(VAO); }
    
    void loadToGPU();

    void draw();

    void loadMeshFromObjFile(std::string filename);

    void printVertices();

private:
    GLuint VAO; // Vertex Array Object (loading attribute pointers)
    GLuint VBO; // Vertex Buffer Object (loading vertices)
    GLuint IBO; // Index Buffer Object (loading indicies to vertices)

    void cleanup();    
};

