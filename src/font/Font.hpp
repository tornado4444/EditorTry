#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "../libraries/stb/stb_image.hpp"
#include <string>
#include <vector>
#include <map>

class Font {
public:
    Font(const char* const drawing);
    ~Font();

public:
    virtual void print(const char* const a_Letters, int const a_X, int const a_Y, float const a_Scale = 1.0f, glm::vec3 a_Color = glm::vec3(1.0f, 1.0f, 1.0f));

    bool isLoaded() const {
        return VAO != 0 || VBO != 0;
    }

private:
    struct Info {
        std::string face;
        size_t size;
        bool bold;
        bool italic;
        uint64_t unicode;
        std::string charSet;
        int stretchH;
        bool smooth;
        int aa;
        int padding[4];
        int spacing[2];
        int outline;
    };

private:
    struct RenderPage {
        unsigned int lineHeight;
        unsigned int base;
        unsigned int scaleW;
        unsigned int scaleH;
        size_t pages;
        bool packed;
        unsigned int alphaChannel;
        unsigned int redChannel;
        unsigned int greenChannel;
        unsigned int blueChannel;
    };

private:
    struct Page {
        size_t id;
        std::string filename;
        GLuint textId = 0;
    };

private:
    struct Character {
        size_t index;
        unsigned int xCoord;
        unsigned int yCoord;
        unsigned int width;
        unsigned int height;
        signed xOffset;
        signed yOffset;
        unsigned int xAdvance;
        unsigned int page;
        unsigned int channel;
        std::map<signed, signed> kerning;
    };

private:
    std::string myFilename;
    Info myInfo;
    RenderPage myPage;
    std::vector<Page> Pages;
    std::map<signed, Character> Chars;

private:
    virtual int getsSubString(const std::string* const a_Input, std::string* a_Output, int a_Start, char a_Prefix = '=', char a_Suffix = ' ');
    virtual void loadFile(const char* const a_FileName);
    virtual void loadTextures();
    virtual void genVertex();

    GLuint VAO, VBO;
};