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

void Mesh::loadMeshFromObjFile(std::string filename) {
    vertices.clear();
    indices.clear();

	std::ifstream f(filename);
    if (!f.is_open()) {
        std::cerr << "Error opening object file: " << filename << "\n";
        return;
    }
    // Local caches for raw vertex data
    std::vector<Vec3> verts;  // Position vertices
    std::vector<Vec2> texs;   // Texture coordinates
    std::vector<Vec3> norms;  // Normals

    // Map to store unique vertices and their indices
    std::map<std::tuple<int, int, int>, GLuint> uniqueVertexMap; // {v, vt, vn} -> index
    GLuint currentIndex = 0;

    std::string line;
    while (std::getline(f, line)) {
        std::stringstream s(line);

        // Skip empty lines or comments
        if (line.empty() || line[0] == '#') continue;

        char junk;

        // Reading vertex data
        if (line[0] == 'v') {
            if (line[1] == 't') { // Texture coordinates (vt)
                Vec2 vt;
                s >> junk >> junk >> vt.u >> vt.v;
                texs.push_back(vt);
            } else if (line[1] == 'n') { // Normals (vn)
                Vec3 vn;
                s >> junk >> junk >> vn.x >> vn.y >> vn.z;
                norms.push_back(vn);
            } else { // Positions (v)
                Vec3 v;
                s >> junk >> v.x >> v.y >> v.z;
                verts.push_back(v);
            }
        }

        // Reading face data
        else if (line[0] == 'f') {
            s >> junk; // Skip 'f'
            std::string token;
            while (std::getline(s, token, ' ')) {
                if (token.empty()) continue;

                std::stringstream faceStream(token);
                std::string vIndexStr, vtIndexStr, vnIndexStr;

                // Parse the face component (v/vt/vn)
                std::getline(faceStream, vIndexStr, '/');
                std::getline(faceStream, vtIndexStr, '/');
                std::getline(faceStream, vnIndexStr, '/');

                // Extract indices (subtract 1 for zero-based indexing)
                int vIndex = vIndexStr.empty() ? -1 : std::stoi(vIndexStr) - 1;
                int vtIndex = vtIndexStr.empty() ? -1 : std::stoi(vtIndexStr) - 1;
                int vnIndex = vnIndexStr.empty() ? -1 : std::stoi(vnIndexStr) - 1;

                // Create a unique key for this vertex
                std::tuple<int, int, int> key = {vIndex, vtIndex, vnIndex};

                // Check if the vertex is already in the map
                if (uniqueVertexMap.find(key) == uniqueVertexMap.end()) {
                    // Add the new unique vertex
                    uniqueVertexMap[key] = currentIndex++;
                    Vertex vertex;

                    // Add the position, texture, and normal to the vertex
                    if (vIndex != -1) vertex.position = verts[vIndex];
                    //if (vtIndex != -1) vertex.texCoord = texs[vtIndex];
                    //if (vnIndex != -1) vertex.normal = norms[vnIndex];

                    // Store the vertex
                    vertices.push_back(vertex);
                }
	
                // Add the index to the indices array
                this->indices.push_back(uniqueVertexMap[key]);
            }
        }
    }

    f.close();
}
