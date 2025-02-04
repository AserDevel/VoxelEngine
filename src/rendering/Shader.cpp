#include <fstream>
#include <sstream>
#include "rendering/Shader.h"


void Shader::use() {
    glUseProgram(programID);
}

void Shader::bindFloat(float f, const char* name) {
    // Get uniform location    
    GLuint uniformLocation = glGetUniformLocation(programID, name);
    
    // Bind uniform
    glUniform1f(uniformLocation, f);
    
    // Check for errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "Error while setting uniform '" << name << "': " << glGetError() << std::endl;
    }
}

void Shader::bindInteger(int i, const char* name) {
    // Get uniform location
    GLuint uniformLocation = glGetUniformLocation(programID, name);

    // Bind uniform
    glUniform1i(uniformLocation, i);
    
    // Check for errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "Error while setting uniform '" << name << "': " << glGetError() << std::endl;
    }
}


void Shader::bindVector2(Vec2 vector, const char* name) {
    // Get uniform location
    GLuint uniformLocation = glGetUniformLocation(programID, name);
    
    // Bind uniform
    glUniform2f(uniformLocation, vector.x, vector.y);
    
    // Check errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "Error while setting uniform '" << name << "': " << glGetError() << std::endl;
    }
}

void Shader::bindVector3(Vec3 vector, const char* name) {
    // Get uniform location
    GLuint uniformLocation = glGetUniformLocation(programID, name);
    
    // Bind uniform
    glUniform3f(uniformLocation, vector.x, vector.y, vector.z);
    
    // Check errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "Error while setting uniform '" << name << "': " << glGetError() << std::endl;
    }
}

void Shader::bindVector4(Vec4 vector, const char* name) {
    // Get uniform location
    GLuint uniformLocation = glGetUniformLocation(programID, name);
    
    // Bind uniform
    glUniform4f(uniformLocation, vector.x, vector.y, vector.z, vector.w);
    
    // Check errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "Error while setting uniform '" << name << "': " << glGetError() << std::endl;
    }
}

void Shader::bindMatrix(Mat4x4 matrix, const char* name) {
    // get uniform location
    GLuint uniformLocation = glGetUniformLocation(programID, name);
    
    // bind matrix uniform
    glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, &matrix[0][0]);

    // check errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "Error while setting uniform '" << name << "': " << glGetError() << std::endl;
    }
}

void Shader::bindMaterials(Material materials[256]) {
    for (size_t i = 0; i < 256; ++i) {
        std::string materialBase = "materials[" + std::to_string(i) + "]";
        glUniform4fv(glGetUniformLocation(programID, (materialBase + ".color").c_str()), 1, &materials[i].color.x);
        glUniform1f(glGetUniformLocation(programID, (materialBase + ".specularity").c_str()), materials[i].specularity);
    }

    GLenum error = glGetError();
	if (error != GL_NO_ERROR) {
		std::cerr << "Error while setting uniform materials: " << error << "\n";
	}
}

void Shader::bindTexture(GLuint textureID, const char* name, int index) {
    // Bind the texture
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Get the uniform location
    GLuint uniformLocation = glGetUniformLocation(programID, name);

    // bind uniform to index
    glUniform1i(uniformLocation, index);
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "Error while setting uniform '" << name <<"': " << error << "\n";
	}
}

void Shader::bindTextureArray(GLuint textureArrayID) {
    // Bind the textureArray    
    glActiveTexture(GL_TEXTURE0); // Use texture unit 0 (first texture)
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureArrayID);

    // Get the uniform location
    GLuint uniformLocation = glGetUniformLocation(programID, "textureArray"); 
    
    // bind uniform to index
    glUniform1i(uniformLocation, 0); // Corresponds to GL_TEXTURE0
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "Error while setting uniform 'textureArray': " << error << "\n";
    }
}

std::string Shader::readFile(const std::string filePath) {    
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filePath);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Function to preprocess the shader and handle #include
std::string Shader::preprocessShader(const std::string& filePath, std::unordered_set<std::string>& processedFiles) {
    // Avoid reprocessing the same file
    if (processedFiles.find(filePath) != processedFiles.end()) {
        return ""; // File already processed
    }
    processedFiles.insert(filePath);

    // Read the main shader file
    std::string shaderCode = readFile(filePath);
    std::stringstream output;
    std::istringstream shaderStream(shaderCode);
    std::string line;

    // Process each line
    while (std::getline(shaderStream, line)) {
        if (line.find("#include") != std::string::npos) {
            // Extract the included file name
            size_t start = line.find("\"") + 1;
            size_t end = line.find("\"", start);
            if (start == std::string::npos || end == std::string::npos) {
                throw std::runtime_error("Malformed #include directive in " + filePath + ": " + line);
            }
            std::string includePath = line.substr(start, end - start);

            // Recursively process the included file
            output << preprocessShader(includePath, processedFiles);
        } else {
            // Regular line, just add it
            output << line << '\n';
        }
    }

    return output.str();
}

std::string Shader::preprocessShader(const std::string& filePath) {
    std::unordered_set<std::string> processedFiles;
    return preprocessShader(filePath, processedFiles);
}

// compile shader program from source code
GLuint Shader::compileShaderProgram(const char* vertexSourceCode, const char* fragmentSourceCode) {
    GLuint vertexShader = 0;
    GLuint fragmentShader = 0;
    int success;
    char infoLog[512];

    // Compile Vertex Shader if provided
    if (vertexSourceCode != nullptr) {
        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexSourceCode, NULL);
        glCompileShader(vertexShader);
        
        // Check for compilation errors
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            std::cerr << "Error compiling vertex shader: " << infoLog << std::endl;
            glDeleteShader(vertexShader);  // Clean up the shader
            return 0;  // Return 0 to indicate error
        }
    } else {
        std::cout << "vertexSourceCode was null" << std::endl;
    }

    // Compile Fragment Shader if provided
    if (fragmentSourceCode != nullptr) {
        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentSourceCode, NULL);
        glCompileShader(fragmentShader);
        
        // Check for compilation errors
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            std::cerr << "Error compiling fragment shader: " << infoLog << std::endl;
            glDeleteShader(fragmentShader);  // Clean up the shader
            if (vertexShader) glDeleteShader(vertexShader); // Cleanup vertex shader if it was compiled
            return 0;  // Return 0 to indicate error
        }
    } else {
        std::cout << "fragmentSourceCode was null" << std::endl;
    }

    // Link shaders into a shader program
    GLuint shaderProgram = glCreateProgram();
    if (vertexShader) glAttachShader(shaderProgram, vertexShader);
    if (fragmentShader) glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Cleanup shaders after linking
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "Error linking shader program: " << infoLog << std::endl;
        glDeleteProgram(shaderProgram); // Cleanup the program on error
        return 0;  // Return 0 to indicate error
    }

    return shaderProgram;
}

// load and compile shader program from file
Shader::Shader(const char* vertexShaderFile) {
    const char* vertexShaderSource = preprocessShader(vertexShaderFile).c_str();

    GLuint compiledProgram = compileShaderProgram(vertexShaderSource, nullptr);
    if (compiledProgram == 0) {
        std::cerr << "Shader compilation or linking failed!" << std::endl;
    }

    this->programID = compiledProgram;
}

// load and compile shader program from file
Shader::Shader(const char* vertexShaderFile, const char* fragmentShaderFile) {
    std::string vertexShaderSource = preprocessShader(vertexShaderFile);
    std::string fragmentShaderSource = preprocessShader(fragmentShaderFile);

    GLuint compiledProgram = compileShaderProgram(vertexShaderSource.c_str(), fragmentShaderSource.c_str());
    if (compiledProgram == 0) {
        std::cerr << "Shader compilation or linking failed with: " << vertexShaderFile << " and " << fragmentShaderFile << std::endl;
    }

    this->programID = compiledProgram;
}