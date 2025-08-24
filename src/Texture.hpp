#pragma once
#include <GL/glew.h>
#include <string>
#include <vector>
#include <cstring>
#include "Shader.hpp"

class Texture {
public:
    GLuint ID;
    GLenum type;
    GLuint unit;
    char path[256];
    char type_r[64];

public:
    Texture() : ID(0), type(GL_TEXTURE_2D), unit(0) {
        path[0] = '\0';
        std::strcpy(type_r, "texture_diffuse");
    }

    Texture(const char* image, GLenum texType, GLuint slot, GLenum format, GLenum pixelType);

    Texture(const char* fullPath, const char* typeName, GLuint textureUnit);

    Texture(const char* pathFile, const char* directory, const char* typeName);

    ~Texture() {
        deleteTexture();
    }

    Texture(const Texture& other)
        : ID(0), type(other.type), unit(other.unit) {
        std::strcpy(path, other.path);
        std::strcpy(type_r, other.type_r);

        ID = other.ID;
    }

    Texture& operator=(const Texture& other) {
        if (this != &other) {
            ID = other.ID;
            type = other.type;
            unit = other.unit;
            std::strcpy(path, other.path);
            std::strcpy(type_r, other.type_r);
        }
        return *this;
    }

    Texture(Texture&& other) noexcept
        : ID(other.ID), type(other.type), unit(other.unit) {
        std::strcpy(path, other.path);
        std::strcpy(type_r, other.type_r);
        other.ID = 0;
        other.path[0] = '\0';
        other.type_r[0] = '\0';
    }

    Texture& operator=(Texture&& other) noexcept {
        if (this != &other) {
            deleteTexture();
            ID = other.ID;
            type = other.type;
            unit = other.unit;
            std::strcpy(path, other.path);
            std::strcpy(type_r, other.type_r);
            other.ID = 0;
            other.path[0] = '\0';
            other.type_r[0] = '\0';
        }
        return *this;
    }

public:
    void texUnit(Shader& shader, const char* uniform, GLuint unit);
    void bind();
    void unbind();
    void deleteTexture() {  
        if (ID != 0) {
            glDeleteTextures(1, &ID);
            ID = 0;
        }
    }

    static unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma = false);
    static unsigned int loadCubemap(std::vector<std::string> faces);
    static unsigned int Texture2D(int w, int h);
    static unsigned int Texture3D(int w, int h, int d);
};