#include <fstream>
#include <sstream>
#include "rendering/Shader.h"


void Shader::use() {
    glUseProgram(programID);
}

void Shader::bindFloat(float f, const char* name) {
    GLuint uniformLocation = glGetUniformLocation(programID, name);
	if (uniformLocation == -1) {
		std::cerr << "Uniform '" << name << "' not found in shader program" << std::endl;
	} else {
		glUniform1f(uniformLocation, f);
		GLenum error = glGetError();
		if (error != GL_NO_ERROR) {
            std::cerr << "Error while setting uniform '" << name << "': " << glGetError() << std::endl;
        }
	}
}

void Shader::bindVector(Vec3 vector, const char* name) {
    GLuint uniformLocation = glGetUniformLocation(programID, name);
	if (uniformLocation == -1) {
		std::cerr << "Uniform '" << name << "' not found in shader program" << std::endl;
	} else {
		glUniform3f(uniformLocation, vector.x, vector.y, vector.z);
		GLenum error = glGetError();
		if (error != GL_NO_ERROR) {
            std::cerr << "Error while setting uniform '" << name << "': " << glGetError() << std::endl;
        }
	}

}

void Shader::bindMatrix(Mat4x4 matrix, const char* name) {
    GLuint uniformLocation = glGetUniformLocation(programID, name);
	if (uniformLocation == -1) {
		std::cerr << "Uniform '" << name << "' not found in shader program" << std::endl;
	} else {
		glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, &matrix[0][0]);
		GLenum error = glGetError();
		if (error != GL_NO_ERROR) {
            std::cerr << "Error while setting uniform '" << name << "': " << glGetError() << std::endl;
        }
	}
}

void Shader::bindMaterials(Material materials[256]) {
    for (size_t i = 0; i < 256; ++i) {
        std::string materialBase = "materials[" + std::to_string(i) + "]";
        glUniform4fv(glGetUniformLocation(programID, (materialBase + ".color").c_str()), 1, &materials[i].color.x);
    }

    GLenum error = glGetError();
	if (error != GL_NO_ERROR) {
		std::cerr << "Error while setting uniform materials: " << error << "\n";
	}
}

void Shader::bindLights(std::vector<LightData> lights) {
    glUniform1i(glGetUniformLocation(programID, "numLights"), lights.size());
    for (size_t i = 0; i < lights.size(); ++i) {
        std::string lightBase = "lights[" + std::to_string(i) + "]";

        glUniform3fv(glGetUniformLocation(programID, (lightBase + ".position").c_str()), 1, &lights[i].position.x);
        glUniform3fv(glGetUniformLocation(programID, (lightBase + ".color").c_str()), 1, &lights[i].color.x);
        glUniform1f(glGetUniformLocation(programID, (lightBase + ".intensity").c_str()), lights[i].intensity);

        glUniform1f(glGetUniformLocation(programID, (lightBase + ".constant").c_str()), lights[i].constant);
        glUniform1f(glGetUniformLocation(programID, (lightBase + ".linear").c_str()), lights[i].linear);
        glUniform1f(glGetUniformLocation(programID, (lightBase + ".quadratic").c_str()), lights[i].quadratic);
    }

    GLenum error = glGetError();
	if (error != GL_NO_ERROR) {
		std::cerr << "Error while setting uniform lights: " << error << "\n";
	}
}

void Shader::bindTexture(GLuint textureID, const char* name, int index) {
    // Bind the texture
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Get the uniform location
    GLuint uniformLocation = glGetUniformLocation(programID, name);
    glUniform1i(uniformLocation, index);
	if (uniformLocation == -1) {
		std::cerr << "Uniform '" << name << "' not found in shader program\n";
	} else {
		glUniform1i(uniformLocation, index);
		GLenum error = glGetError();
		if (error != GL_NO_ERROR) {
			std::cerr << "Error while setting uniform '" << name <<"': " << error << "\n";
		}
	}
}

void Shader::bindTextureArray(GLuint textureArrayID) {
    glActiveTexture(GL_TEXTURE0); // Use texture unit 0 (first texture)
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureArrayID);

    GLuint uniformLocation = glGetUniformLocation(programID, "textureArray");
	if (uniformLocation == -1) {
		std::cerr << "Uniform 'textureArray' not found in shader program\n";
	} else {
		glUniform1i(uniformLocation, 0); // Corresponds to GL_TEXTURE0
		GLenum error = glGetError();
		if (error != GL_NO_ERROR) {
			std::cerr << "Error while setting uniform 'textureArray': " << error << "\n";
		}
	}
}

// Function to load and split shaders
std::unordered_map<std::string, std::string> Shader::loadShadersFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open shader file: " + filePath);
    }

    enum ShaderType { NONE = -1, VERTEX, FRAGMENT };
    ShaderType currentType = NONE;

    std::unordered_map<std::string, std::string> shaderSources;
    shaderSources["vertex"] = "";
    shaderSources["fragment"] = "";

    std::string line;
    std::stringstream ss[2]; // Two streams for vertex and fragment shaders

    while (std::getline(file, line)) {
        if (line.find("#shader") != std::string::npos) {
            if (line.find("vertex") != std::string::npos) {
                currentType = VERTEX;
            } else if (line.find("fragment") != std::string::npos) {
                currentType = FRAGMENT;
            }
        } else {
            if (currentType != NONE) {
                ss[currentType] << line << '\n';
            }
        }
    }

    shaderSources["vertex"] = ss[VERTEX].str();
    shaderSources["fragment"] = ss[FRAGMENT].str();
    
    return shaderSources;
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
Shader::Shader(const char* shaderFile) {
    std::unordered_map<std::string, std::string> shaders = loadShadersFromFile(shaderFile);

    const char* vertexShaderSource = shaders["vertex"].c_str();
    const char* fragmentShaderSource = shaders["fragment"].c_str();

    GLuint compiledProgram = compileShaderProgram(vertexShaderSource, fragmentShaderSource);
    if (compiledProgram == 0) {
        std::cerr << "Shader compilation or linking failed!" << std::endl;
    }

    this->programID = compiledProgram;
}