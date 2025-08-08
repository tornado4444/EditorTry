#include "Texture.hpp"
#include <iostream>
#include <filesystem>
#include <cstring>
#include "../../libraries/stb/stb_image.hpp"

Texture::Texture(const char* image, GLenum texType, GLuint slot, GLenum format, GLenum pixelType) {
    type = texType;
    unit = slot;
    ID = 0;

    strncpy(path, image, sizeof(path) - 1);
    path[sizeof(path) - 1] = '\0';

    strncpy(type_r, "texture_diffuse", sizeof(type_r) - 1);
    type_r[sizeof(type_r) - 1] = '\0';

    int widthImg, heightImg, numColch;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* bytes = stbi_load(image, &widthImg, &heightImg, &numColch, 0);

    if (bytes) {
        GLenum internalFormat;
        GLenum dataFormat = format;

        if (numColch == 1) {
            internalFormat = GL_RED;
            if (format == GL_RGBA) dataFormat = GL_RED;
        }
        else if (numColch == 3) {
            internalFormat = GL_RGB;
            if (format == GL_RGBA) dataFormat = GL_RGB;
        }
        else if (numColch == 4) {
            internalFormat = GL_RGBA;
        }
        else {
            internalFormat = GL_RGB;
        }

        glGenTextures(1, &ID);
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(texType, ID);

        glTexParameteri(texType, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        glTexParameteri(texType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(texType, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(texType, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexImage2D(texType, 0, internalFormat, widthImg, heightImg, 0, dataFormat, pixelType, bytes);
        glGenerateMipmap(texType);

        stbi_image_free(bytes);
        glBindTexture(texType, 0);
    }
    else {
        std::cerr << "Failed to load texture: " << image << std::endl;
        ID = 0;
    }
}

Texture::Texture(const char* fullPath, const char* typeName, GLuint textureUnit) {
    type = GL_TEXTURE_2D;
    unit = textureUnit;
    ID = 0;

    strncpy(path, fullPath, sizeof(path) - 1);
    path[sizeof(path) - 1] = '\0';

    strncpy(type_r, typeName, sizeof(type_r) - 1);
    type_r[sizeof(type_r) - 1] = '\0';

    ID = TextureFromFile(fullPath, "", false);
}

Texture::Texture(const char* pathFile, const char* directory, const char* typeName) {
    type = GL_TEXTURE_2D;
    unit = 0;
    ID = 0;

    strncpy(this->path, pathFile, sizeof(this->path) - 1);
    this->path[sizeof(this->path) - 1] = '\0';

    strncpy(type_r, typeName, sizeof(type_r) - 1);
    type_r[sizeof(type_r) - 1] = '\0';

    std::string filename = directory;
    filename += '/';
    filename += pathFile;

    ID = TextureFromFile(filename.c_str(), "", false);
}

void Texture::texUnit(Shader& shader, const char* uniform, GLuint unit) {
    GLuint textUnit = glGetUniformLocation(shader.ID, uniform);
    shader.use();
    glUniform1i(textUnit, unit);
}

unsigned int Texture::TextureFromFile(const char* path, const std::string& directory, bool gamma) {
    std::string filename = path;
    if (!directory.empty()) {
        filename = directory + '/' + filename;
    }

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);

    if (data) {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;
        else {
            format = GL_RGB;
            std::cerr << "Unusual texture format with " << nrComponents << " components at: " << path << std::endl;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else {
        std::cerr << "Texture failed to load at path: " << path << std::endl;
        textureID = 0;
    }

    return textureID;
}

unsigned int Texture::loadCubemap(std::vector<std::string> faces) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else {
            std::cerr << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

unsigned int Texture::Texture2D(int w, int h) {
    unsigned int tex_output;
    glGenTextures(1, &tex_output);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_output);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);

    glBindImageTexture(0, tex_output, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    return tex_output;
}

unsigned int Texture::Texture3D(int w, int h, int d) {
    unsigned int tex_output;
    glGenTextures(1, &tex_output);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, tex_output);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, w, h, d, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(0, tex_output, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    return tex_output;
}

void Texture::bind() {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(type, ID);
}

void Texture::unbind() {
    glBindTexture(type, 0);
}
