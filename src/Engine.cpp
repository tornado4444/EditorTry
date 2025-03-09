#include "Engine.hpp"
#include "stb_image.hpp"

// .glm files
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>

Engine::Engine() : Window() {
    glEnable(GL_DEPTH_TEST);
    camera = new Camera(glm::vec3(0.0f, 1.0f, 5.0f));
    lastX = getWindowWidth() / 2.0f;
    lastY = getWindowHeight() / 2.0f;
    firstClick = true;
    glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Engine::initialize() {
    shader = new Shader("../../../shaders/default.vert", "../../../shaders/default.frag");
    textRender = new Shader("../../../shaders/textShader.vert", "../../../shaders/textShader.frag");

    // Initialize VAO, VBO, EBO
    vao = new VAO();
    vbo = new VBO(vertices, sizeof(vertices), GL_STATIC_DRAW);
    ebo = new EBO(indices, sizeof(indices), GL_STATIC_DRAW);

    vao->Bind();
    vbo->Bind();
    ebo->Bind();

    vao->linkAttrib(*vbo, 0, 3, GL_FLOAT, 8 * sizeof(float), (void*)0);                   // COORDINATES
    vao->linkAttrib(*vbo, 1, 3, GL_FLOAT, 8 * sizeof(float), (void*)(3 * sizeof(float))); // Colors
    vao->linkAttrib(*vbo, 2, 2, GL_FLOAT, 8 * sizeof(float), (void*)(6 * sizeof(float))); // Tex Coordinates

    vao->UnBind();
    vbo->UnBind();
    ebo->UnBind();

    // FOR THE TEXTURE
    std::string texturePath = "textures/sand.png";
    texture = new Texture;
    textureID = texture->TextureFromFile("sand.png", "textures", false);
    if (textureID == 0) {
        MyglobalLogger().logMessage(Logger::ERROR, "Failed to load texture at path: " + texturePath, __FILE__, __LINE__);
        return;
    }

    // Initialize font
    font = new Font("../../../Fonts/comicSans_32.fnt");
    if (!font) {
        MyglobalLogger().logMessage(Logger::ERROR, "Failed to initialize font!", __FILE__, __LINE__);
        return;
    }

    shader->use();
    shader->setInt("tex0", 0);

    glfwSetWindowUserPointer(getWindow(), this);
    glfwSetCursorPosCallback(getWindow(), [](GLFWwindow* window, double xpos, double ypos) {
        Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
        engine->cursorPosCallback(window, xpos, ypos);
    });
    glfwSetScrollCallback(getWindow(), [](GLFWwindow* window, double xoffset, double yoffset) {
        Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
        engine->scroll_callback(window, xoffset, yoffset);
    });
}

Engine::~Engine() {
    delete shader;
    delete vao, vbo, ebo;
    delete textRender;
    delete font;
    delete texture;
    delete camera;
}

void Engine::processInput(GLFWwindow* window) {
    static float lastFrame = 0.0f;
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera->ProcessKeyboard(FORWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera->ProcessKeyboard(BACKWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera->ProcessKeyboard(LEFT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera->ProcessKeyboard(RIGHT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        camera->ProcessKeyboard(UP, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        camera->ProcessKeyboard(DOWN, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

void Engine::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    if (firstClick) {
        lastX = xpos;
        lastY = ypos;
        firstClick = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    camera->ProcessMouseMovement(xoffset, yoffset);
}

void Engine::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera->ProcessMouseScroll(yoffset);
}


void Engine::render() {
    // Clear the buffer and color
    glClearColor(0.3f, 0.6f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int width = widthWindow, height = heightWindow;
    glfwGetWindowSize(getWindow(), &width, &height);

    // Use the shader to the pyramide
    shader->use();

    // Matrix prohection and view
    projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
    view = camera->getViewMatrix();
    glm::mat4 camMatrix = projection * view;

    float angle = (float)glfwGetTime() * 50.0f;
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // Располагаем в центре
    model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f)); // Вращаем
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f)); // Нормальный масштаб

    // Uniform variable
    shader->setMat4("model", model);
    shader->setMat4("camMatrix", camMatrix);

    // VAO, VBO, EBO
    vao->Bind();
    vbo->Bind();
    ebo->Bind();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);

    int indexCount = sizeof(indices) / sizeof(indices[0]);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);

    vao->UnBind();
    vbo->UnBind();
    ebo->UnBind();

    textRender->use();

    glm::mat4 textProjection = glm::ortho(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, -1.0f, 1.0f);
    textRender->setMat4("projection", textProjection);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    std::string text = "OpenGL Vendor: " + std::string(reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
    font->print(text.c_str(), 10.0f, 60.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));

    std::string text1 = "OpenGL Renderer: " + std::string(reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
    font->print(text1.c_str(), 10.0f, 40.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));

    std::string debugText = "Camera Position: " +
        std::to_string(camera->Position.x) + ", " +
        std::to_string(camera->Position.y) + ", " +
        std::to_string(camera->Position.z);
    font->print(debugText.c_str(), 10.0f, 20.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
    std::string FPSstring = "FPS: " + std::to_string(1.0f / deltaTime) + "\n";
    font->print(FPSstring.c_str(), 10.0f, 85.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
    glDisable(GL_BLEND);
}