#pragma once

#include <gl/glew.h>
#include <glfw/glfw3.h>
#include <string>
#include <cstdio>
#include <memory>
#include <filesystem>

#include "../src/Logger/Logger.hpp"
#include "Shader.hpp"
#include "../src/font/Font.hpp"
#include "VAO.hpp"
#include "VBO.hpp"
#include "EBO.hpp"
#include "Texture.hpp"
#include "Window.hpp"
#include "Camera.hpp"
#include "Model.hpp"


class Init : public Window {
public:
    Init();
    ~Init() = default;

    Init(const Init&) = delete;
    Init& operator=(const Init&) = delete;
    Init(Init&&) = default;
    Init& operator=(Init&&) = default;

public:
    virtual void initialize();
    virtual void render();

public:
    void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    void processInput(GLFWwindow* window);

private:
    std::unique_ptr<Camera> camera;
    std::unique_ptr<Shader> shader;
    std::unique_ptr<Shader> textRender;
    std::unique_ptr<Font> font;
    std::unique_ptr<Texture> texture;
    std::unique_ptr<Model> model;

private:
    glm::mat4 projection;
    glm::mat4 view;
    std::string filename;
    unsigned int textureID = 0;
    float deltaTime = 0.0f;
    bool firstClick = true;
    float lastX, lastY;
    float aspectRatio = 1.0f;
};
