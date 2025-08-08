#include "../font/Font.hpp"
#include "../src/Logger/Logger.hpp"
#include "Shader.hpp"
#include "../libraries/stb/stb_image.hpp"
#include <iostream>
#include <fstream>
#include <set>

#pragma warning(once : 6397)

using std::ifstream;
using std::string;
using std::cout;

Font::Font(const char* const drawing) {
    myFilename = drawing;
    loadFile(drawing);
    loadTextures();
    genVertex();
    MyglobalLogger().logMessage(Logger::DEBUG, "Initialize the font for text.", __FILE__, __LINE__);
}

Font::~Font() {
    for (int i = 0; i < Pages.size(); i++) {
        glDeleteTextures(1, &Pages[i].textId);
        Pages[i].textId = 0;
    }

    if (VAO != 0) {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }
}

int Font::getsSubString(const std::string* const a_Input, std::string* a_Output, int a_Start, char a_Prefix, char a_Suffix) {
    int start = a_Input->find_first_of(a_Prefix, a_Start) + 1;
    int end = a_Input->find_first_of(a_Suffix, start);
    a_Output->assign(a_Input->substr(start, end - start));

    return end;
}

void Font::loadFile(const char* const a_FileName) {
    string line;
    ifstream read = ifstream(a_FileName);

    bool complete = true;
    signed ourLine = 0;
    signed stage = 0;

    signed currentLeft = 0;

    if (read.is_open()) {
        while (std::getline(read, line)) {
            ourLine++;

            string temp;
            size_t end = 0;

            if (stage == 0) {
                //face
                end = getsSubString(&line, &temp, end, '"', '"');
                myInfo.face = temp;

                //size
                end = getsSubString(&line, &temp, end);
                myInfo.size = stoi(temp, nullptr, 10);

                //bold
                end = getsSubString(&line, &temp, end);
                myInfo.bold = stoi(temp, nullptr, 10) ? true : false;

                //char set
                //unused
                end = getsSubString(&line, &temp, end, '"', '"');

                //unicode
                //unused
                end = getsSubString(&line, &temp, end);

                //stretch
                end = getsSubString(&line, &temp, end);
                myInfo.stretchH = stoi(temp, nullptr, 10);

                //smooth
                end = getsSubString(&line, &temp, end);
                myInfo.smooth = stoi(temp, nullptr, 10);

                //aa
                end = getsSubString(&line, &temp, end);
                myInfo.aa = stoi(temp, nullptr, 10);

                //padding
                end = getsSubString(&line, &temp, end, '=', ',');
                myInfo.padding[0] = stoi(temp, nullptr, 10);
                end = getsSubString(&line, &temp, end, ',', ',');
                myInfo.padding[1] = stoi(temp, nullptr, 10);
                end = getsSubString(&line, &temp, end, ',', ',');
                myInfo.padding[2] = stoi(temp, nullptr, 10);
                end = getsSubString(&line, &temp, end, ',', ' ');
                myInfo.padding[3] = stoi(temp, nullptr, 10);

                //spacing
                end = getsSubString(&line, &temp, end, '=', ',');
                myInfo.spacing[0] = stoi(temp, nullptr, 10);
                end = getsSubString(&line, &temp, end, ',', ' ');
                myInfo.spacing[1] = stoi(temp, nullptr, 10);

                //outline
                int start = line.find_last_of('=', line.length()) + 1;
                temp = line.substr(start, end - line.length());
                myInfo.outline = stoi(temp, nullptr, 10);

                stage++;
            }
            else if (stage == 1) {//common
                //line height
                end = getsSubString(&line, &temp, end);
                myPage.lineHeight = stoi(temp, nullptr, 10);

                //base
                end = getsSubString(&line, &temp, end);
                myPage.base = stoi(temp, nullptr, 10);

                //scaleW
                end = getsSubString(&line, &temp, end);
                myPage.scaleW = stoi(temp, nullptr, 10);

                //scaleH
                end = getsSubString(&line, &temp, end);
                myPage.scaleH = stoi(temp, nullptr, 10);

                //pages
                end = getsSubString(&line, &temp, end);
                currentLeft = stoi(temp, nullptr, 10);
                myPage.pages = currentLeft;

                //packed
                end = getsSubString(&line, &temp, end);
                myPage.packed = stoi(temp, nullptr, 10);

                //color channels
                //a
                end = getsSubString(&line, &temp, end);
                myPage.alphaChannel = stoi(temp, nullptr, 10);
                //r
                end = getsSubString(&line, &temp, end);
                myPage.redChannel = stoi(temp, nullptr, 10);
                //g
                end = getsSubString(&line, &temp, end);
                myPage.greenChannel = stoi(temp, nullptr, 10);
                //b
                int start = line.find_last_of('=', line.length()) + 1;
                temp = line.substr(start, end - line.length());
                myPage.blueChannel = stoi(temp, nullptr, 10);

                stage++;

            }
            else if (stage == 2) {//pages
                Page page;

                //id
                end = getsSubString(&line, &temp, end);
                page.id = stoi(temp, nullptr, 10);

                //file name/texture name
                end = getsSubString(&line, &temp, end, '"', '"');
                page.filename = temp;

                //add to vector
                Pages.push_back(page);

                //one less of this stage to do
                currentLeft--;
                if (currentLeft <= 0) {
                    stage++;
                }

            }
            else if (stage == 3) {//char amount
                int start = line.find_last_of('=', line.length()) + 1;
                temp = line.substr(start, end - line.length());
                currentLeft = stoi(temp, nullptr, 10);

                stage++;
            }
            else if (stage == 4) {//char area
                Character letter;

                //id
                end = getsSubString(&line, &temp, end);
                letter.index = stoi(temp, nullptr, 10);

                //x
                end = getsSubString(&line, &temp, end);
                letter.xCoord = stoi(temp, nullptr, 10);

                //y
                end = getsSubString(&line, &temp, end);
                letter.yCoord = stoi(temp, nullptr, 10);

                //width
                end = getsSubString(&line, &temp, end);
                letter.width = stoi(temp, nullptr, 10);

                //height
                end = getsSubString(&line, &temp, end);
                letter.height = stoi(temp, nullptr, 10);

                //xOffset
                end = getsSubString(&line, &temp, end);
                letter.xOffset = stoi(temp, nullptr, 10);

                //yOffset
                end = getsSubString(&line, &temp, end);
                letter.yOffset = stoi(temp, nullptr, 10);

                //xAdvance
                end = getsSubString(&line, &temp, end);
                letter.xAdvance = stoi(temp, nullptr, 10);

                //page
                end = getsSubString(&line, &temp, end);
                letter.page = stoi(temp, nullptr, 10);

                //channel
                int start = line.find_last_of('=', line.length()) + 1;
                temp = line.substr(start, end - line.length());
                letter.channel = stoi(temp, nullptr, 10);

                //add to map
                Chars[letter.index] = letter;

                //one less of this stage to do
                currentLeft--;
                if (currentLeft <= 0) {
                    stage++;
                }
            }
            else if (stage == 5) {//kerning amount
                int start = line.find_last_of('=', line.length()) + 1;
                temp = line.substr(start, end - line.length());
                currentLeft = stoi(temp, nullptr, 10);

                stage++;
            }
            else if (stage == 6) {//kerning area
                int first, second, amount;

                //left character
                end = getsSubString(&line, &temp, end);
                first = stoi(temp, nullptr, 10);

                //right character
                end = getsSubString(&line, &temp, end);
                second = stoi(temp, nullptr, 10);

                //amount
                int start = line.find_last_of('=', line.length()) + 1;
                temp = line.substr(start, end - line.length());
                amount = stoi(temp, nullptr, 10);

                //add to map
                Chars.at(first).kerning[second] = amount;

                //one less of this stage to do
                currentLeft--;
                if (currentLeft <= 0) {
                    stage++;
                }
            }
        }

        read.close();
    }
    else {
        complete = false;
    }

    if (!complete) {
        MyglobalLogger().logMessage(Logger::ERROR, "Failed reading font: " + std::string(a_FileName) + " stuck at line: " + std::to_string(ourLine), __FILE__, __LINE__);
    }
}

void Font::loadTextures() {
    string baseFolder = myFilename.substr(0, myFilename.find_last_of('/', myFilename.length()) + 1);

    for (int i = 0; i < Pages.size(); i++) {
        string filename = baseFolder + Pages[i].filename;

        GLenum err = glGetError();
        glGenTextures(1, &Pages[i].textId);
        if (err != GL_NO_ERROR) {
            MyglobalLogger().logMessage(Logger::ERROR, "OpenGL error after glGenTextures: " + std::to_string(err), __FILE__, __LINE__);
        }

        glBindTexture(GL_TEXTURE_2D, Pages[i].textId);

        if (err != GL_NO_ERROR) {
            MyglobalLogger().logMessage(Logger::ERROR, "OpenGL error after glGenTextures: " + std::to_string(err), __FILE__, __LINE__);
        }

        // Parameters for texture rendering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int width, height, channels;
        unsigned char* image = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);

        if (!image) {
            MyglobalLogger().logMessage(Logger::ERROR, "Failed to load texture: " + filename, __FILE__, __LINE__);
            continue;
        }

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        stbi_image_free(image);
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    MyglobalLogger().logMessage(Logger::DEBUG, "Loading font texture from: " + myFilename, __FILE__, __LINE__);
}

void Font::genVertex() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Font::print(const char* const a_Letters, int const a_X, int const a_Y, float const a_Scale, glm::vec3 a_Color) {
    if (!VAO || !VBO) {
        MyglobalLogger().logMessage(Logger::ERROR, "ERROR TO INITIALIZE TO VAO AND VBO!", __FILE__, __LINE__);
        return;
    }

    if (!a_Letters || strlen(a_Letters) == 0) {
        return;
    }

    GLint currentProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);

    if (currentProgram == 0) {
        MyglobalLogger().logMessage(Logger::ERROR, "No shader program is currently bound!", __FILE__, __LINE__);
        return;
    }

    GLint colorLoc = glGetUniformLocation(currentProgram, "Color");
    GLint imageLoc = glGetUniformLocation(currentProgram, "image");

    if (colorLoc != -1) {
        glUniform3f(colorLoc, a_Color.x, a_Color.y, a_Color.z);
    }

    if (imageLoc != -1) {
        glUniform1i(imageLoc, 0);
    }

    glBindVertexArray(this->VAO);

    if (Pages.empty()) {
        MyglobalLogger().logMessage(Logger::ERROR, "No font texture pages loaded!", __FILE__, __LINE__);
        glBindVertexArray(0);
        return;
    }

    float imgHeight = static_cast<float>(myPage.scaleH);
    float imgWidth = static_cast<float>(myPage.scaleW);

    float xPos = a_X;
    float yPos = a_Y;

    int index = 0;
    int maxIndex = strlen(a_Letters);

    // Debug output for font information
    static bool debugPrinted = false;
    if (!debugPrinted) {
        std::cout << "\n=== FONT DEBUG FIXED ===" << std::endl;
        std::cout << "Text to render: \"" << std::string(a_Letters).substr(0, std::min(20, maxIndex)) << "\"" << std::endl;
        std::cout << "Font texture size: " << imgWidth << "x" << imgHeight << std::endl;
        std::cout << "Line height: " << myPage.lineHeight << ", Base: " << myPage.base << std::endl;

        // Debug first few characters
        for (int i = 0; i < std::min(5, maxIndex); i++) {
            unsigned char c = static_cast<unsigned char>(a_Letters[i]);
            auto it = Chars.find(c);
            if (it != Chars.end()) {
                const Character& ch = it->second;
                std::cout << "Char '" << (char)c << "' (id=" << (int)c << "): "
                    << "pos=(" << ch.xCoord << "," << ch.yCoord << ") "
                    << "size=" << ch.width << "x" << ch.height << " "
                    << "offset=(" << ch.xOffset << "," << ch.yOffset << ") "
                    << "advance=" << ch.xAdvance << std::endl;
            }
        }
        debugPrinted = true;
        std::cout << "=========================" << std::endl;
    }

    while (index < maxIndex && a_Letters[index] != '\0') {
        unsigned char currentChar = static_cast<unsigned char>(a_Letters[index]);

        if (currentChar == '\n') {
            yPos += myPage.lineHeight * a_Scale;
            xPos = a_X;
            index++;
            continue;
        }

        auto iteratorLetter = Chars.find(currentChar);
        if (iteratorLetter != Chars.end()) {
            const Character& letter = iteratorLetter->second;

            if (letter.width <= 0 || letter.height <= 0) {
                // Even for empty characters, advance the position
                xPos += letter.xAdvance * a_Scale;
                index++;
                continue;
            }

            float sizeWidth = letter.width * a_Scale;
            float sizeHeight = letter.height * a_Scale;

            // FIXED UV COORDINATES:
            // 1. Normalize coordinates properly
            float uvLeft = static_cast<float>(letter.xCoord) / imgWidth;
            float uvRight = static_cast<float>(letter.xCoord + letter.width) / imgWidth;
            float uvTop = static_cast<float>(letter.yCoord) / imgHeight;
            float uvBottom = static_cast<float>(letter.yCoord + letter.height) / imgHeight;

            // 2. Calculate character position with offsets
            float charLeft = xPos + (letter.xOffset * a_Scale);
            float charRight = charLeft + sizeWidth;
            float charTop = yPos + (letter.yOffset * a_Scale);
            float charBottom = charTop + sizeHeight;

            // 3. CORRECT vertices for quad (counter-clockwise)
            GLfloat verts[6][4] = {
                // First triangle (top-left -> bottom-right -> top-right)
                { charLeft,  charTop,    uvLeft,  uvTop    },     // top-left
                { charRight, charBottom, uvRight, uvBottom },     // bottom-right  
                { charRight, charTop,    uvRight, uvTop    },     // top-right

                // Second triangle (top-left -> bottom-left -> bottom-right)
                { charLeft,  charTop,    uvLeft,  uvTop    },     // top-left
                { charLeft,  charBottom, uvLeft,  uvBottom },     // bottom-left
                { charRight, charBottom, uvRight, uvBottom }      // bottom-right
            };

            // Check page validity
            if (letter.page < 0 || letter.page >= Pages.size()) {
                MyglobalLogger().logMessage(Logger::WARNING, "Invalid page index for character: " + std::to_string(currentChar), __FILE__, __LINE__);
                index++;
                continue;
            }

            // Bind texture
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, Pages[letter.page].textId);

            // Update buffer
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);

            // Draw
            glDrawArrays(GL_TRIANGLES, 0, 6);

            // Advance position
            xPos += letter.xAdvance * a_Scale;

            // Kerning
            if (index + 1 < maxIndex && a_Letters[index + 1] != '\0') {
                unsigned char nextChar = static_cast<unsigned char>(a_Letters[index + 1]);
                auto kerningIt = letter.kerning.find(nextChar);
                if (kerningIt != letter.kerning.end()) {
                    xPos += kerningIt->second * a_Scale;
                }
            }

        }
        else {
            // Character not found
            static std::set<unsigned char> reportedMissing;
            if (reportedMissing.find(currentChar) == reportedMissing.end()) {
                std::cout << "Missing char: " << (int)currentChar << " ('" << (char)currentChar << "')" << std::endl;
                reportedMissing.insert(currentChar);
            }
            xPos += myPage.base * a_Scale * 0.3f; // Smaller spacing for unknown characters
        }

        index++;
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}