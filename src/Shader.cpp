#include "Shader.hpp"

Shader::Shader(const char* vertexPath, const char* fragmentPath) {
    try {
        std::cout << "=== SHADER CREATION DEBUG ===" << std::endl;
        std::cout << "Vertex shader path: " << vertexPath << std::endl;
        std::cout << "Fragment shader path: " << fragmentPath << std::endl;

        std::string vShaderCode = loadShaderFromFile(vertexPath);
        std::string fShaderCode = loadShaderFromFile(fragmentPath);

        const char* vShaderString = vShaderCode.c_str();
        const char* fShaderString = fShaderCode.c_str();

        unsigned int vertex, fragment;

        // Vertex shader
        std::cout << "Compiling vertex shader..." << std::endl;
        vertex = glCreateShader(GL_VERTEX_SHADER);
        if (vertex == 0) {
            throw std::runtime_error("Failed to create vertex shader object");
        }

        glShaderSource(vertex, 1, &vShaderString, NULL);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX", getShaderName(vertexPath));

        // Fragment shader
        std::cout << "Compiling fragment shader..." << std::endl;
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        if (fragment == 0) {
            glDeleteShader(vertex); // Очистка
            throw std::runtime_error("Failed to create fragment shader object");
        }

        glShaderSource(fragment, 1, &fShaderString, NULL);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT", getShaderName(fragmentPath));

        // Shader program
        std::cout << "Creating shader program..." << std::endl;
        ID = glCreateProgram();
        if (ID == 0) {
            glDeleteShader(vertex);
            glDeleteShader(fragment);
            throw std::runtime_error("Failed to create shader program");
        }

        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM", "");

        // Clean up shaders
        glDeleteShader(vertex);
        glDeleteShader(fragment);

        std::cout << "Shader program created successfully with ID: " << ID << std::endl;
        std::cout << "=============================" << std::endl;

        // Log successful compilation
        MyglobalLogger().logMessage(Logger::DEBUG,
            "SHADERS " + getShaderName(vertexPath) + " AND " + getShaderName(fragmentPath) + " LOADED AND COMPILED!",
            __FILE__, __LINE__);
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
        }
    }
}

std::string Shader::loadShaderFromFile(const char* shaderPath) {
    std::string shaderCode;
    std::ifstream shaderFile;

    // Сначала проверим путь без исключений
    std::cout << "Attempting to load shader: " << shaderPath << std::endl;

    // Проверяем существование файла без выброса исключений
    std::ifstream testFile(shaderPath);
    if (!testFile.good()) {
        std::string error = "Shader file does not exist or cannot be accessed: " + std::string(shaderPath);
        MyglobalLogger().logMessage(Logger::ERROR, error, __FILE__, __LINE__);
        throw std::invalid_argument(error);
    }
    testFile.close();

    // Теперь устанавливаем исключения ПОСЛЕ проверки
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

bool Shader::createFromString(const char* vertexSource, const char* fragmentSource) {
    unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexSource, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX", "string");

    unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentSource, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT", "string");

    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM", "string");

    glDeleteShader(vertex);
    glDeleteShader(fragment);
    return true;
}

// Static uniform setters
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
    std::cout << "\n=== SHADER " << ID << " UNIFORMS ===" << std::endl;

    GLint numUniforms = 0;
    glGetProgramiv(ID, GL_ACTIVE_UNIFORMS, &numUniforms);

    for (GLint i = 0; i < numUniforms; i++) {
        GLint size;
        GLenum type;
        GLchar uniformName[256];
        glGetActiveUniform(ID, i, sizeof(uniformName), nullptr, &size, &type, uniformName);
        std::cout << "  - " << uniformName << " (type: " << type << ")" << std::endl;
    }
    std::cout << "==============================" << std::endl;
}