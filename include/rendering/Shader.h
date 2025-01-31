#pragma once

#include "glad/glad.h"
#include "utilities/standard.h"
#include "world/Voxel.h"
#include "unordered_set"

class Shader {
    GLuint programID;

    std::string readFile(const std::string filePath);

    std::string preprocessShader(const std::string& filePath, std::unordered_set<std::string>& processedFiles);

    std::string preprocessShader(const std::string& filePath);
    
    GLuint compileShaderProgram(const char* vertexSourceCode, const char* fragmentSourceCode);

public:
    Shader() = default;
    Shader(const char* vertexShaderFile);
    Shader(const char* vertexShaderFile, const char* fragmentShaderFile);

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

    void bindFloat(float f, const char* name);

    void bindInteger(int i, const char* name);

    void bindVector2(Vec2 vector, const char* name);

    void bindVector3(Vec3 vector, const char* name);

    void bindVector4(Vec4 vector, const char* name);

    void bindTexture(GLuint textureID, const char* name, int index);

    void bindTextureArray(GLuint textureArrayID);

    void bindMaterials(Material materials[256]);
};