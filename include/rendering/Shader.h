#pragma once

#include "glad/glad.h"
#include "utilities/standard.h"
#include "world/Voxels.h"

struct LightData {
    Vec3 position;
    Vec3 color;
    float intensity;

    // Attenuation factors
    float constant = 1;
    float linear;
    float quadratic;
};

class Shader {
    GLuint programID;

    std::unordered_map<std::string, std::string> loadShadersFromFile(const std::string& filePath);

    GLuint compileShaderProgram(const char* vertexSourceCode, const char* fragmentSourceCode);

public:
    Shader(const char* shaderFile);

    ~Shader() {
        if (programID) {
            glUseProgram(0);
            glDeleteProgram(programID);
            programID = 0;
        } 
    }

    void use();

    // Uniform binding functions
    void bindMatrix(Mat4x4 matrix, const char* name);

    void bindTexture(GLuint textureID, const char* name, int index);

    void bindTextureArray(GLuint textureArrayID);

    void bindVector(Vec3 vector, const char* name);

    void bindFloat(float f, const char* name);

    void bindMaterials(Material materials[256]);

    void bindLights(std::vector<LightData> lights);
};