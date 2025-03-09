#pragma once

#include <gl/glew.h>
#include <glfw/glfw3.h>
#include <string>
#include <cstdio>
#include <filesystem>

// .hpp files
#include "../src/Logger/Logger.hpp"
#include "Shader.hpp"
#include "../src/font/Font.hpp"
#include "VAO.hpp"
#include "VBO.hpp"
#include "EBO.hpp"
#include "../../Game/Texture.hpp"
#include "Window.hpp"

class Engine : public Window {
public:
    Engine();
    ~Engine();

public:
    virtual void initialize();
    virtual void render();

public:
    void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    void processInput(GLFWwindow* window);

private:
    Camera* camera = nullptr;

private:
    Shader* shader = nullptr;
    Shader* textRender = nullptr;

private:
    Font* font = nullptr;
    glm::mat4 projection;
    glm::mat4 view;
    std::string filename;

private:
    VAO* vao = nullptr;
    VBO* vbo = nullptr;
    EBO* ebo = nullptr;

    // Textures
private:
    Texture* texture = nullptr;
    unsigned int textureID = 0;
    float deltaTime = 0.0f;
    bool firstClick = true;
    float lastX, lastY;

private:
    static constexpr GLfloat vertices[] = {
        // COORDINATES           /   COLORS             /   NORMALISE
         -0.5f, 0.0f,   0.5f,     0.83f, 0.70f, 0.44f,	0.0f, 0.0f,
         -0.5f, 0.0f,  -0.5f,     0.83f, 0.70f, 0.44f,	0.0f, 1.0f,
          0.5f, 0.0f,  -0.5f,     0.83f, 0.70f, 0.44f,	1.0f, 1.0f,
          0.5f, 0.0f,   0.5f,     0.83f, 0.70f, 0.44f,	1.0f, 0.0f,
          0.0f, 0.8f,   0.0f,     0.92f, 0.86f, 0.76f,	0.5f, 0.5f
    };

    static constexpr GLint indices[] = {
        0, 1, 2,
        0, 2, 3,
        0, 1, 4,
        1, 2, 4,
        2, 3, 4,
        3, 0, 4
    };
};