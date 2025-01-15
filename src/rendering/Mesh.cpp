#include "rendering/Mesh.h"
#include <fstream>
#include <sstream>
#include <tuple>
#include <map>

const Vertex cubeVertices[6][4] {
    // +X face
    {
        { Vec3(0.5f, -0.5f, -0.5f),  0 },
        { Vec3(0.5f, 0.5f, 0.5f),    0 },
        { Vec3(0.5f, 0.5f, -0.5f),   0 },
        { Vec3(0.5f, -0.5f, 0.5f),   0 },
    },
    // -X face
    {
        { Vec3(-0.5f, -0.5f, 0.5f),  0 },
        { Vec3(-0.5f, 0.5f, -0.5f),  0 },
        { Vec3(-0.5f, 0.5f, 0.5f),   0 },
        { Vec3(-0.5f, -0.5f, -0.5f), 0 },
    },
    // +Z face
    {
        { Vec3(0.5f, -0.5f, 0.5f),   0 },
        { Vec3(-0.5f, 0.5f, 0.5f),   0 },
        { Vec3(0.5f, 0.5f, 0.5f),    0 },
        { Vec3(-0.5f, -0.5f, 0.5f),  0 },
    },
    // -Z face
    {
        { Vec3(-0.5f, -0.5f, -0.5f), 0 },
        { Vec3(0.5f, 0.5f, -0.5f),   0 },
        { Vec3(-0.5f, 0.5f, -0.5f),  0 },
        { Vec3(0.5f, -0.5f, -0.5f),  0 },
    },
    // +Y face
    {
        { Vec3(-0.5f, 0.5f, -0.5f),  0 },
        { Vec3(0.5f, 0.5f, 0.5f),    0 },
        { Vec3(-0.5f, 0.5f, 0.5f),   0 },
        { Vec3(0.5f, 0.5f, -0.5f),   0 },
    },
    // -Y face
    {
        { Vec3(0.5f, -0.5f, 0.5f),   0 },
        { Vec3(-0.5f, -0.5f, -0.5f), 0 },
        { Vec3(-0.5f, -0.5f, 0.5f),  0 },
        { Vec3(0.5f, -0.5f, -0.5f),  0 },
    },
};

const Vec3 cubeNormals[6] = {
    Vec3(1, 0, 0),  Vec3(-1, 0, 0),  // +X, -X
    Vec3(0, 0, 1),  Vec3(0, 0, -1),  // +Z, -Z
    Vec3(0, 1, 0),  Vec3(0, -1, 0),  // +Y, -Y
};

const GLuint cubeIndicies[6] = { 0, 1, 2, 0, 3, 1 };

const GLuint cubeIndiciesFlipped[6] = { 0, 3, 2, 3, 1, 2 };

Mesh::Mesh() {
    // Generate buffers and bind to VAO
	glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &IBO);

    GLenum error = glGetError();
    if (error != 0 || !VAO || !VBO || !IBO) {
        std::cerr << "Error during buffer initialization: " << error << std::endl;
    }
}

void Mesh::cleanup() {
    vertices.clear();
	indices.clear();
    
    // delete buffers
	if (VBO) glDeleteBuffers(1, &VBO);
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (IBO) glDeleteBuffers(1, &IBO);
    
    // Unbind buffers
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    VBO = VAO = IBO = 0;
}

void Mesh::loadToGPU() {
    if (vertices.empty()) {
        std::cerr << "Attempted to upload empty vertex buffer" << std::endl;
        return;
    }

    if (!VAO || !VBO) {
        std::cerr << "Error: buffers are not initialized" << std::endl;
        return;
    }
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

	GLenum error = glGetError();
	if (error != GL_NO_ERROR) {
		std::cerr << "Error during buffer data upload: " << error << "\n";
	}

    // Define vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(0));
    glEnableVertexAttribArray(0);
    glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(Vertex), (void*)(sizeof(Vec3)));
    glEnableVertexAttribArray(1);

	error = glGetError();
	if (error != GL_NO_ERROR) {
		std::cerr << "Error during VAO setup: " << error << "\n";
	}

    // Generate and bind Index Buffer Object (IBO)
    if (!indices.empty()) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
    }

    // Unbind buffers
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// Draw triangles
void Mesh::draw() {
    if (vertices.empty()) {
        std::cerr << "Attempted to draw empty vertex buffer" << std::endl;
        return;
    }

    if (!indices.empty()) {
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, indices.data());
    } else {
        glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    }
}
