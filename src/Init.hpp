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
#include "Menu.hpp"
#include "LBVH.hpp"

class Init : public Window {
public:
    Init();
    ~Init();

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
    bool initializeEnvironmentResources();
    void renderEnvironment(int width, int height, float timeSeconds);
    void destroyEnvironmentResources();
    static std::filesystem::path resolveResourcePath(const std::string& relativePath);
    static GLuint loadTexture2DForAtmosphere(const std::filesystem::path& path);
    static GLuint loadTexture3DFromAtlas(const std::filesystem::path& path);
private:
    std::unique_ptr<Camera> camera;
    std::unique_ptr<Shader> shader;
    std::unique_ptr<Shader> environmentShader;
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
    GLuint atmosphereVAO = 0;
    GLuint atmosphereVBO = 0;
    GLuint lowFrequencyTex3D = 0;
    GLuint highFrequencyTex3D = 0;
    GLuint weatherTex2D = 0;
    GLuint curlNoiseTex2D = 0;
    GLuint moonTex2D = 0;
    GLuint starTex2D = 0;
    bool hasMoonTexture = false;
    bool hasStarTexture = false;
    bool atmosphereReady = false;

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
