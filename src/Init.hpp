#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
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
#include "Menu.hpp"
#include "LBVH.hpp"

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
    void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    bool rayAABBIntersectWorld(const glm::vec3& origin, const glm::vec3& dir,
        const glm::vec3& boxMin, const glm::vec3& boxMax);
private:
    std::unique_ptr<Camera> camera;
    std::unique_ptr<Shader> shader;
    std::unique_ptr<Shader> normalsShader;
    std::unique_ptr<Shader> textRender;
    std::unique_ptr<Shader> aabbShader;
    std::unique_ptr<Shader> mortonShader;
    std::unique_ptr<Shader> sortShader;
    std::unique_ptr<Shader> hierarchyShader;
    std::unique_ptr<Shader> lbvhAABBShader;
    std::unique_ptr<Font> font;
    std::unique_ptr<Texture> texture;
    std::unique_ptr<Model> model;
    std::unique_ptr<Menu> menu;
    std::unique_ptr<BVH> bvh;
    GLuint cubeVAO, cubeVBO, cubeEBO;

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