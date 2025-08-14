// Shader.hpp
#pragma once
#include <gl/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>
#include <string>
#include <set>
#include <fstream>
#include <sstream>
#include <iostream>
#include "../src/Logger/Logger.hpp"

static GLuint m_ProgramInUse = 0;

class Shader {
public:
    Shader() : ID(0) {}

    // Конструктор с опциональным геометрическим шейдером
    Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr);

    virtual ~Shader();

public:
    unsigned int ID = 0;

    unsigned int getId() const {
        return ID;
    }

    virtual void use() const;
    virtual void Activate() const { use(); }

    // Методы для установки uniform переменных
    virtual void setBool(const std::string& name, bool value) const;
    virtual void setInt(const std::string& name, int value) const;
    virtual void setFloat(const std::string& name, float value) const;
    virtual void setVec2(const std::string& name, glm::vec2 vector) const;
    virtual void setVec3(const std::string& name, glm::vec3 vector) const;
    virtual void setVec4(const std::string& name, glm::vec4 vector) const;
    virtual void setMat4(const std::string& name, glm::mat4 matrix) const;
    virtual void setSampler2D(const std::string& name, unsigned int texture, int id) const;
    virtual void setSampler3D(const std::string& name, unsigned int texture, int id) const;

    // Статические методы для установки uniform (используют m_ProgramInUse)
    static void setUniform(const char* a_Uniform, const GLfloat a_V0, const GLfloat a_V1, const GLfloat a_V2);
    static void setUniform(const char* a_Uniform, const GLfloat a_V0, const GLfloat a_V1, const GLfloat a_V2, const GLfloat a_V3);
    static void setUniform(const char* a_Uniform, const GLfloat a_V0);
    static void setUniform(const char* a_Uniform, const GLuint a_V0);
    static void setUniform(const char* a_Uniform, const GLint a_V0);
    static void setUniform(const char* a_Uniform, const glm::mat4 a_V0);
    static void setUniform(const char* a_Uniform, const GLfloat a_V0, const GLfloat a_V1);
    static void setUniform(const char* a_Uniform, const GLint a_V0, const GLint a_V1);
    static void setProjection(const glm::mat4 a_Projection);

    // Утилиты
    void debugUniforms() const;
    bool isCompiled() const {
        return ID != 0;
    }

protected:
    virtual void checkCompileErrors(unsigned int shader, std::string type, std::string shaderName);
    virtual std::string getShaderName(const char* shaderPath);
    virtual std::string loadShaderFromFile(const char* shaderPath);

    virtual bool createFromString(const char* vertexSource, const char* fragmentSource, const char* geometrySource = nullptr);

    virtual bool createFromString(const char* vertexSource, const char* fragmentSource);
};