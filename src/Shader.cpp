#include "Shader.hpp"

Shader::Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath) {
    try {
        if (geometryPath != nullptr) {
            std::cout << "Geometry shader path: " << geometryPath << std::endl;
        }

        std::string vShaderCode = loadShaderFromFile(vertexPath);
        std::string fShaderCode = loadShaderFromFile(fragmentPath);
        std::string gShaderCode;

        bool hasGeometryShader = (geometryPath != nullptr && strlen(geometryPath) > 0);
        if (hasGeometryShader) {
            gShaderCode = loadShaderFromFile(geometryPath);
        }

        const char* vShaderString = vShaderCode.c_str();
        const char* fShaderString = fShaderCode.c_str();
        const char* gShaderString = hasGeometryShader ? gShaderCode.c_str() : nullptr;

        unsigned int vertex, fragment, geometry = 0;

        std::cout << "Compiling vertex shader..." << std::endl;
        vertex = glCreateShader(GL_VERTEX_SHADER);
        if (vertex == 0) {
            throw std::runtime_error("Failed to create vertex shader object");
        }

        glShaderSource(vertex, 1, &vShaderString, NULL);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX", getShaderName(vertexPath));

        std::cout << "Compiling fragment shader..." << std::endl;
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        if (fragment == 0) {
            glDeleteShader(vertex);
            throw std::runtime_error("Failed to create fragment shader object");
        }

        glShaderSource(fragment, 1, &fShaderString, NULL);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT", getShaderName(fragmentPath));

        if (hasGeometryShader) {
            std::cout << "Compiling geometry shader..." << std::endl;
            geometry = glCreateShader(GL_GEOMETRY_SHADER);
            if (geometry == 0) {
                glDeleteShader(vertex);
                glDeleteShader(fragment);
                throw std::runtime_error("Failed to create geometry shader object");
            }

            glShaderSource(geometry, 1, &gShaderString, NULL);
            glCompileShader(geometry);
            checkCompileErrors(geometry, "GEOMETRY", getShaderName(geometryPath));
        }

        std::cout << "Creating shader program..." << std::endl;
        ID = glCreateProgram();
        if (ID == 0) {
            glDeleteShader(vertex);
            glDeleteShader(fragment);
            if (hasGeometryShader) glDeleteShader(geometry);
            throw std::runtime_error("Failed to create shader program");
        }

        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        if (hasGeometryShader) {
            glAttachShader(ID, geometry);
        }

        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM", "");

        glDeleteShader(vertex);
        glDeleteShader(fragment);
        if (hasGeometryShader) {
            glDeleteShader(geometry);
        }

        std::cout << "Shader program created successfully with ID: " << ID << std::endl;
        if (hasGeometryShader) {
            std::cout << "Geometry shader included!" << std::endl;
        }

        std::string logMessage = "SHADERS " + getShaderName(vertexPath) + " AND " + getShaderName(fragmentPath);
        if (hasGeometryShader) {
            logMessage += " AND " + getShaderName(geometryPath);
        }
        logMessage += " LOADED AND COMPILED!";

        MyglobalLogger().logMessage(Logger::DEBUG, logMessage, __FILE__, __LINE__);
    }
    catch (const std::invalid_argument& e) {
        std::cerr << "std::invalid_argument in Shader constructor: " << e.what() << std::endl;
        MyglobalLogger().logMessage(Logger::ERROR, "std::invalid_argument: " + std::string(e.what()), __FILE__, __LINE__);
        throw;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in Shader constructor: " << e.what() << std::endl;
        MyglobalLogger().logMessage(Logger::ERROR, "Shader creation failed: " + std::string(e.what()), __FILE__, __LINE__);
        throw;
    }
}

Shader::Shader(const char* computePath) {
    try {
        std::cout << "=== COMPUTE SHADER CREATION DEBUG ===" << std::endl;
        std::cout << "Compute shader path: " << computePath << std::endl;

        std::string computeCode = loadShaderFromFile(computePath);
        const char* computeString = computeCode.c_str();

        unsigned int compute = glCreateShader(GL_COMPUTE_SHADER);
        if (compute == 0) {
            throw std::runtime_error("Failed to create compute shader object");
        }

        glShaderSource(compute, 1, &computeString, NULL);
        glCompileShader(compute);
        checkCompileErrors(compute, "COMPUTE", getShaderName(computePath));

        // Создаём программу
        ID = glCreateProgram();
        if (ID == 0) {
            glDeleteShader(compute);
            throw std::runtime_error("Failed to create compute shader program");
        }

        glAttachShader(ID, compute);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM", getShaderName(computePath));

        glDeleteShader(compute);

        std::cout << "Compute shader program created successfully with ID: " << ID << std::endl;
        MyglobalLogger().logMessage(Logger::DEBUG, "COMPUTE SHADER " + getShaderName(computePath) + " LOADED AND COMPILED!", __FILE__, __LINE__);
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in Compute Shader constructor: " << e.what() << std::endl;
        MyglobalLogger().logMessage(Logger::ERROR, "Compute shader creation failed: " + std::string(e.what()), __FILE__, __LINE__);
        throw;
    }
}

void Shader::dispatchCompute(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z) const {
    if (ID == 0) {
        MyglobalLogger().logMessage(Logger::ERROR, "Cannot dispatch compute shader: invalid program ID", __FILE__, __LINE__);
        return;
    }
    glUseProgram(ID);
    glDispatchCompute(num_groups_x, num_groups_y, num_groups_z);
    // Добавим барьер для синхронизации (важно для LBVH)
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

Shader::~Shader() {
    if (ID != 0) {
        glDeleteProgram(ID);
        ID = 0;
    }
    MyglobalLogger().logMessage(Logger::DEBUG, "Shader program deleted.", __FILE__, __LINE__);
}

void Shader::use() const {
    glUseProgram(ID);
    m_ProgramInUse = ID;
}

void Shader::setBool(const std::string& name, bool value) const {
    GLint location = glGetUniformLocation(ID, name.c_str());
    glUniform1i(location, (int)value);
}

void Shader::setInt(const std::string& name, int value) const {
    GLint location = glGetUniformLocation(ID, name.c_str());
    if (location == -1) {
        MyglobalLogger().logMessage(Logger::WARNING,
            "Failed to find uniform: " + name, __FILE__, __LINE__);
        return;
    }
    glUniform1i(location, value);
}

void Shader::setFloat(const std::string& name, float value) const {
    GLint location = glGetUniformLocation(ID, name.c_str());
    if (location == -1) {
        MyglobalLogger().logMessage(Logger::WARNING,
            "Failed to find uniform: " + name, __FILE__, __LINE__);
        return;
    }
    glUniform1f(location, value);
}

void Shader::setVec2(const std::string& name, glm::vec2 vector) const {
    GLint location = glGetUniformLocation(ID, name.c_str());
    if (location == -1) {
        MyglobalLogger().logMessage(Logger::WARNING,
            "Failed to find uniform: " + name, __FILE__, __LINE__);
        return;
    }
    glUniform2fv(location, 1, glm::value_ptr(vector));
}

void Shader::setVec3(const std::string& name, glm::vec3 vector) const {
    GLint location = glGetUniformLocation(ID, name.c_str());
    if (location == -1) {
        MyglobalLogger().logMessage(Logger::WARNING,
            "Failed to find uniform: " + name, __FILE__, __LINE__);
        return;
    }
    glUniform3fv(location, 1, glm::value_ptr(vector));
}

void Shader::setVec4(const std::string& name, glm::vec4 vector) const {
    GLint location = glGetUniformLocation(ID, name.c_str());
    if (location == -1) {
        MyglobalLogger().logMessage(Logger::WARNING,
            "Failed to find uniform: " + name, __FILE__, __LINE__);
        return;
    }
    glUniform4fv(location, 1, glm::value_ptr(vector));
}

void Shader::setMat4(const std::string& name, glm::mat4 matrix) const {
    GLint currentProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);

    if (currentProgram != (GLint)ID) {
        MyglobalLogger().logMessage(Logger::WARNING,
            "Trying to set uniform '" + name + "' on inactive shader! Current: " +
            std::to_string(currentProgram) + ", Expected: " + std::to_string(ID), __FILE__, __LINE__);
        return;
    }

    GLint location = glGetUniformLocation(ID, name.c_str());
    if (location == -1) {
        static std::set<std::string> reportedUniforms;
        std::string key = std::to_string(ID) + "_" + name;
        if (reportedUniforms.find(key) == reportedUniforms.end()) {
            MyglobalLogger().logMessage(Logger::INFO,
                "Uniform '" + name + "' not found in shader " + std::to_string(ID), __FILE__, __LINE__);
            reportedUniforms.insert(key);
        }
        return;
    }
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
}

void Shader::setSampler2D(const std::string& name, unsigned int texture, int id) const {
    glActiveTexture(GL_TEXTURE0 + id);
    glBindTexture(GL_TEXTURE_2D, texture);
    this->setInt(name, id);
}

void Shader::setSampler3D(const std::string& name, unsigned int texture, int id) const {
    glActiveTexture(GL_TEXTURE0 + id);
    glBindTexture(GL_TEXTURE_3D, texture);
    this->setInt(name, id);
}

void Shader::checkCompileErrors(unsigned int shader, std::string type, std::string shaderName) {
    int success;
    char infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::ostringstream logMessage;
            logMessage << "ERROR: SHADER " << shaderName
                << " COMPILATION ERROR of type: " << type
                << "\n" << infoLog
                << "\n-------------------------------------------------------";
            MyglobalLogger().logMessage(Logger::ERROR, logMessage.str(), __FILE__, __LINE__);
            throw std::runtime_error("Shader compilation failed: " + type + " " + shaderName);
        }
    }
    else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::ostringstream logMessage;
            logMessage << "ERROR::PROGRAM_LINKING_ERROR " << shaderName
                << " of type: " << type
                << "\n" << infoLog
                << "\n-------------------------------------------------------";
            MyglobalLogger().logMessage(Logger::ERROR, logMessage.str(), __FILE__, __LINE__);
            throw std::runtime_error("Shader program linking failed");
        }
    }
}

std::string Shader::loadShaderFromFile(const char* shaderPath) {
    std::string shaderCode;
    std::ifstream shaderFile;

    std::cout << "Attempting to load shader: " << shaderPath << std::endl;

    std::ifstream testFile(shaderPath);
    if (!testFile.good()) {
        std::string error = "Shader file does not exist or cannot be accessed: " + std::string(shaderPath);
        MyglobalLogger().logMessage(Logger::ERROR, error, __FILE__, __LINE__);
        throw std::invalid_argument(error);
    }
    testFile.close();

    shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        shaderFile.open(shaderPath);

        if (!shaderFile.is_open()) {
            throw std::runtime_error("Failed to open shader file: " + std::string(shaderPath));
        }

        std::stringstream shaderStream;
        shaderStream << shaderFile.rdbuf();
        shaderFile.close();
        shaderCode = shaderStream.str();

        if (shaderCode.empty()) {
            throw std::runtime_error("Shader file is empty: " + std::string(shaderPath));
        }

        std::cout << "Successfully loaded shader: " << shaderPath << " (size: " << shaderCode.length() << " bytes)" << std::endl;

    }
    catch (const std::ifstream::failure& e) {
        std::string error = "std::ifstream::failure when reading shader " + std::string(shaderPath) + ": " + e.what();
        MyglobalLogger().logMessage(Logger::ERROR, error, __FILE__, __LINE__);
        throw std::invalid_argument(error);
    }
    catch (const std::exception& e) {
        std::string error = "Exception when reading shader " + std::string(shaderPath) + ": " + e.what();
        MyglobalLogger().logMessage(Logger::ERROR, error, __FILE__, __LINE__);
        throw;
    }

    return shaderCode;
}

std::string Shader::getShaderName(const char* path) {
    std::string pathstr = std::string(path);
    const size_t last_slash_idx = pathstr.find_last_of("/");
    if (std::string::npos != last_slash_idx) {
        pathstr.erase(0, last_slash_idx + 1);
    }
    return pathstr;
}

bool Shader::createFromString(const char* vertexSource, const char* fragmentSource, const char* geometrySource) {
    try {
        unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vertexSource, NULL);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX", "string");

        unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fragmentSource, NULL);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT", "string");

        unsigned int geometry = 0;
        bool hasGeometryShader = (geometrySource != nullptr && strlen(geometrySource) > 0);

        if (hasGeometryShader) {
            geometry = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(geometry, 1, &geometrySource, NULL);
            glCompileShader(geometry);
            checkCompileErrors(geometry, "GEOMETRY", "string");
        }

        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        if (hasGeometryShader) {
            glAttachShader(ID, geometry);
        }

        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM", "string");

        glDeleteShader(vertex);
        glDeleteShader(fragment);
        if (hasGeometryShader) {
            glDeleteShader(geometry);
        }

        return true;
    }
    catch (const std::exception& e) {
        MyglobalLogger().logMessage(Logger::ERROR, "createFromString failed: " + std::string(e.what()), __FILE__, __LINE__);
        return false;
    }
}

bool Shader::createFromString(const char* vertexSource, const char* fragmentSource) {
    return createFromString(vertexSource, fragmentSource, nullptr);
}

void Shader::setUniform(const char* a_Uniform, const GLfloat a_V0, const GLfloat a_V1, const GLfloat a_V2) {
    GLint location = glGetUniformLocation(m_ProgramInUse, a_Uniform);
    if (location != -1) {
        glUniform3f(location, a_V0, a_V1, a_V2);
    }
}

void Shader::setUniform(const char* a_Uniform, const GLfloat a_V0, const GLfloat a_V1, const GLfloat a_V2, const GLfloat a_V3) {
    GLint location = glGetUniformLocation(m_ProgramInUse, a_Uniform);
    if (location != -1) {
        glUniform4f(location, a_V0, a_V1, a_V2, a_V3);
    }
}

void Shader::setUniform(const char* a_Uniform, const GLfloat a_V0) {
    GLint location = glGetUniformLocation(m_ProgramInUse, a_Uniform);
    if (location != -1) {
        glUniform1f(location, a_V0);
    }
}

void Shader::setUniform(const char* a_Uniform, const GLuint a_V0) {
    GLint location = glGetUniformLocation(m_ProgramInUse, a_Uniform);
    if (location != -1) {
        glUniform1i(location, a_V0);
    }
}

void Shader::setUniform(const char* a_Uniform, const GLint a_V0) {
    GLint location = glGetUniformLocation(m_ProgramInUse, a_Uniform);
    if (location != -1) {
        glUniform1i(location, a_V0);
    }
}

void Shader::setUniform(const char* a_Uniform, const glm::mat4 a_V0) {
    GLint location = glGetUniformLocation(m_ProgramInUse, a_Uniform);
    if (location != -1) {
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(a_V0));
    }
}

void Shader::setUniform(const char* a_Uniform, const GLfloat a_V0, const GLfloat a_V1) {
    GLint location = glGetUniformLocation(m_ProgramInUse, a_Uniform);
    if (location != -1) {
        glUniform2f(location, a_V0, a_V1);
    }
}

void Shader::setUniform(const char* a_Uniform, const GLint a_V0, const GLint a_V1) {
    GLint location = glGetUniformLocation(m_ProgramInUse, a_Uniform);
    if (location != -1) {
        glUniform2i(location, a_V0, a_V1);
    }
}

void Shader::setProjection(const glm::mat4 a_Projection) {
    GLint location = glGetUniformLocation(m_ProgramInUse, "projection");
    if (location != -1) {
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(a_Projection));
    }
}

void Shader::debugUniforms() const {
    GLint numUniforms = 0;
    glGetProgramiv(ID, GL_ACTIVE_UNIFORMS, &numUniforms);

    for (GLint i = 0; i < numUniforms; i++) {
        GLint size;
        GLenum type;
        GLchar uniformName[256];
        glGetActiveUniform(ID, i, sizeof(uniformName), nullptr, &size, &type, uniformName);
    }
}