#include "Init.hpp"

#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>

Init::Init() : Window() {
    glEnable(GL_DEPTH_TEST);

    camera = std::make_unique<Camera>(glm::vec3(0.0f, 0.0f, 2.0f));

    lastX = getWindowWidth() / 2.0f;
    lastY = getWindowHeight() / 2.0f;
    firstClick = true;
    glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Init::initialize() {
    shader = std::make_unique<Shader>("../../../shaders/default.vert", "../../../shaders/default.frag");
    textRender = std::make_unique<Shader>("../../../shaders/textShader.vert", "../../../shaders/textShader.frag");

    if (!shader || !textRender) {
        MyglobalLogger().logMessage(Logger::ERROR, "Failed to load shaders!", __FILE__, __LINE__);
        return;
    }

    try {
        model = std::make_unique<Model>("../../../models/bunny/scene.gltf");
        MyglobalLogger().logMessage(Logger::INFO, "Successfully loaded GLTF model: scene.gltf", __FILE__, __LINE__);

        if (model && !model->meshes.empty()) {
            std::cout << "\n=== MODEL DIAGNOSTIC ===" << std::endl;
            std::cout << "Total meshes: " << model->meshes.size() << std::endl;

            for (size_t i = 0; i < model->meshes.size(); ++i) {
                const auto& mesh = model->meshes[i];
                std::cout << "Mesh " << i << ":" << std::endl;
                std::cout << "  Vertices: " << mesh.vertices.size() << std::endl;
                std::cout << "  Indices: " << mesh.indices.size() << std::endl;

                if (!mesh.vertices.empty()) {
                    glm::vec3 minBounds(FLT_MAX);
                    glm::vec3 maxBounds(-FLT_MAX);

                    for (const auto& vertex : mesh.vertices) {
                        minBounds = glm::min(minBounds, vertex.position);
                        maxBounds = glm::max(maxBounds, vertex.position);
                    }

                    std::cout << "  Min bounds: (" << minBounds.x << ", " << minBounds.y << ", " << minBounds.z << ")" << std::endl;
                    std::cout << "  Max bounds: (" << maxBounds.x << ", " << maxBounds.y << ", " << maxBounds.z << ")" << std::endl;

                    glm::vec3 size = maxBounds - minBounds;
                    std::cout << "  Size: (" << size.x << ", " << size.y << ", " << size.z << ")" << std::endl;
                }
            }
            std::cout << "=========================" << std::endl;
        }

    }
    catch (const std::exception& e) {
        MyglobalLogger().logMessage(Logger::ERROR, "Failed to load GLTF model: " + std::string(e.what()), __FILE__, __LINE__);
        return;
    }

    font = std::make_unique<Font>("../../../Fonts/comicSans_32.fnt");
    if (!font) {
        MyglobalLogger().logMessage(Logger::ERROR, "Failed to initialize font!", __FILE__, __LINE__);
        return;
    }

    shader->use();

    glfwSetWindowUserPointer(getWindow(), this);
    glfwSetCursorPosCallback(getWindow(), [](GLFWwindow* window, double xpos, double ypos) {
        Init* engine = static_cast<Init*>(glfwGetWindowUserPointer(window));
        engine->cursorPosCallback(window, xpos, ypos);
        });
    glfwSetScrollCallback(getWindow(), [](GLFWwindow* window, double xoffset, double yoffset) {
        Init* engine = static_cast<Init*>(glfwGetWindowUserPointer(window));
        engine->scroll_callback(window, xoffset, yoffset);
        });
}

void Init::processInput(GLFWwindow* window) {
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

void Init::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
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

void Init::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera->ProcessMouseScroll(yoffset);
}

void Init::render() {
    glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int width = widthWindow, height = heightWindow;
    glfwGetWindowSize(getWindow(), &width, &height);
    glViewport(0, 0, width, height);

    shader->use();

    projection = glm::perspective(glm::radians(camera->Zoom), (float)width / (float)height, 0.1f, 100.0f);
    view = camera->getViewMatrix();

    glm::mat4 modelMatrix = glm::mat4(1.0f);

    glm::vec3 modelPosition = glm::vec3(0.0f, 0.0f, -8.0f);

    static float debugRotationX = 90.0f;
    static float debugRotationY = 180.0f; 
    static float debugRotationZ = 1000.0f;
    static float bunnyScale = 10000.0f; 

    static bool key1Pressed = false, key2Pressed = false;
    static bool key3Pressed = false, key4Pressed = false;
    static bool key5Pressed = false, key6Pressed = false;
    static bool key7Pressed = false, key8Pressed = false;

    bool key1Current = (glfwGetKey(getWindow(), GLFW_KEY_1) == GLFW_PRESS);
    bool key2Current = (glfwGetKey(getWindow(), GLFW_KEY_2) == GLFW_PRESS);
    bool key3Current = (glfwGetKey(getWindow(), GLFW_KEY_3) == GLFW_PRESS);
    bool key4Current = (glfwGetKey(getWindow(), GLFW_KEY_4) == GLFW_PRESS);
    bool key5Current = (glfwGetKey(getWindow(), GLFW_KEY_5) == GLFW_PRESS);
    bool key6Current = (glfwGetKey(getWindow(), GLFW_KEY_6) == GLFW_PRESS);
    bool key7Current = (glfwGetKey(getWindow(), GLFW_KEY_7) == GLFW_PRESS);
    bool key8Current = (glfwGetKey(getWindow(), GLFW_KEY_8) == GLFW_PRESS);

    if (key1Current && !key1Pressed) { debugRotationX += 15.0f; std::cout << "Rotation X: " << debugRotationX << std::endl; }
    if (key2Current && !key2Pressed) { debugRotationX -= 15.0f; std::cout << "Rotation X: " << debugRotationX << std::endl; }
    if (key3Current && !key3Pressed) { debugRotationY += 15.0f; std::cout << "Rotation Y: " << debugRotationY << std::endl; }
    if (key4Current && !key4Pressed) { debugRotationY -= 15.0f; std::cout << "Rotation Y: " << debugRotationY << std::endl; }
    if (key5Current && !key5Pressed) { debugRotationZ += 15.0f; std::cout << "Rotation Z: " << debugRotationZ << std::endl; }
    if (key6Current && !key6Pressed) { debugRotationZ -= 15.0f; std::cout << "Rotation Z: " << debugRotationZ << std::endl; }
    if (key7Current && !key7Pressed) { bunnyScale += 0.1f; std::cout << "Scale: " << bunnyScale << std::endl; }
    if (key8Current && !key8Pressed) { bunnyScale -= 0.1f; std::cout << "Scale: " << bunnyScale << std::endl; }

    key1Pressed = key1Current; key2Pressed = key2Current;
    key3Pressed = key3Current; key4Pressed = key4Current;
    key5Pressed = key5Current; key6Pressed = key6Current;
    key7Pressed = key7Current; key8Pressed = key8Current;

    if (bunnyScale < 0.1f) bunnyScale = 0.1f;
    if (bunnyScale > 20.0f) bunnyScale = 20.0f;

    modelMatrix = glm::translate(modelMatrix, modelPosition);

    modelMatrix = glm::rotate(modelMatrix, glm::radians(debugRotationX), glm::vec3(1.0f, 0.0f, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(debugRotationY), glm::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(debugRotationZ), glm::vec3(0.0f, 0.0f, 1.0f));

    modelMatrix = glm::scale(modelMatrix, glm::vec3(bunnyScale));

    shader->setMat4("view", view);
    shader->setMat4("projection", projection);
    shader->setMat4("model", modelMatrix);
    shader->setVec3("camPos", camera->Position);

    GLint timeLocation = glGetUniformLocation(shader->ID, "time");
    if (timeLocation != -1) {
        shader->setFloat("time", (float)glfwGetTime());
    }

    GLint diffuseLocation = glGetUniformLocation(shader->ID, "diffuse");
    if (diffuseLocation != -1) {
        shader->setInt("diffuse", 0);
    }

    GLint specularLocation = glGetUniformLocation(shader->ID, "specular");
    if (specularLocation != -1) {
        shader->setInt("specular", 1);
    }

    static bool wireframe = false;
    static bool tKeyPressed = false;
    bool tKeyCurrentlyPressed = (glfwGetKey(getWindow(), GLFW_KEY_T) == GLFW_PRESS);
    if (tKeyCurrentlyPressed && !tKeyPressed) {
        wireframe = !wireframe;
        std::cout << "WIREFRAME MODE " << (wireframe ? "ON" : "OFF") << std::endl;
    }
    tKeyPressed = tKeyCurrentlyPressed;

    if (wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(2.0f);
        glDisable(GL_BLEND);
    }
    else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);
    }

    if (model && !model->meshes.empty()) {
        glDisable(GL_CULL_FACE);

        try {
            shader->use();
            model->Draw(*shader, *camera);
        }
        catch (const std::exception& e) {
            std::cout << "Exception during model draw: " << e.what() << std::endl;
        }

        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cout << "OpenGL error after model draw: " << error << std::endl;
        }
    }

    glDepthMask(GL_TRUE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_BLEND);

    textRender->use();

    glm::mat4 textProjection = glm::ortho(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, -1.0f, 1.0f);
    textRender->setMat4("projection", textProjection);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    if (font) {
        try {
            textRender->setVec3("Color", glm::vec3(1.0f, 1.0f, 0.0f));
            textRender->setInt("image", 0);

            std::string text = "OpenGL Vendor: " + std::string(reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
            font->print(text.c_str(), 10.0f, 60.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));

            std::string text1 = "OpenGL Renderer: " + std::string(reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
            font->print(text1.c_str(), 10.0f, 40.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));

            std::string debugText = "Camera Position: " +
                std::to_string(camera->Position.x) + ", " +
                std::to_string(camera->Position.y) + ", " +
                std::to_string(camera->Position.z);
            font->print(debugText.c_str(), 10.0f, 20.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));

            std::string FPSstring = "FPS: " + std::to_string(1.0f / deltaTime);
            font->print(FPSstring.c_str(), 10.0f, 85.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));

            if (model && !model->meshes.empty()) {
                textRender->setVec3("Color", glm::vec3(0.0f, 0.8f, 1.0f));
                std::string meshInfo = "Glass Bunny | Meshes: " + std::to_string(model->meshes.size()) +
                    " | Vertices: " + std::to_string(model->meshes[0].vertices.size());
                font->print(meshInfo.c_str(), 10.0f, 130.0f, 1.0f, glm::vec3(0.0f, 0.8f, 1.0f));

                textRender->setVec3("Color", glm::vec3(0.8f, 1.0f, 0.8f));
                std::string glassInfo = "GLASS EFFECT ACTIVE";
                font->print(glassInfo.c_str(), 10.0f, 105.0f, 1.0f, glm::vec3(0.8f, 1.0f, 0.8f));

                std::string rotInfo = "Rotations X:" + std::to_string((int)debugRotationX) +
                    " Y:" + std::to_string((int)debugRotationY) +
                    " Z:" + std::to_string((int)debugRotationZ) +
                    " Scale:" + std::to_string(bunnyScale);
                font->print(rotInfo.c_str(), 10.0f, 155.0f, 0.8f, glm::vec3(1.0f, 1.0f, 0.0f));
            }
            else {
                textRender->setVec3("Color", glm::vec3(1.0f, 0.0f, 0.0f));
                std::string noModelText = "No model loaded - showing fallback points";
                font->print(noModelText.c_str(), 10.0f, 105.0f, 0.7f, glm::vec3(1.0f, 0.0f, 0.0f));
            }

            if (wireframe) {
                textRender->setVec3("Color", glm::vec3(1.0f, 0.5f, 0.0f));
                std::string wireframeText = "WIREFRAME MODE ON";
                font->print(wireframeText.c_str(), 10.0f, 180.0f, 1.0f, glm::vec3(1.0f, 0.5f, 0.0f));
            }

            textRender->setVec3("Color", glm::vec3(0.7f, 0.7f, 0.7f));
            std::string controlsText = "Controls: WASD+Mouse | Space/Shift-Up/Down | T-Wireframe | 1-8 Rotate/Scale | ESC-Exit";
            font->print(controlsText.c_str(), 10.0f, static_cast<float>(height - 30), 0.5f, glm::vec3(0.7f, 0.7f, 0.7f));

            std::string debugInstructions = "DEBUG: 1/2-RotX | 3/4-RotY | 5/6-RotZ | 7/8-Scale";
            font->print(debugInstructions.c_str(), 10.0f, static_cast<float>(height - 50), 0.5f, glm::vec3(0.0f, 1.0f, 1.0f));

        }
        catch (const std::exception& e) {
            static int errorCount = 0;
            if (errorCount % 300 == 0) {
                std::cout << "Font rendering error: " << e.what() << std::endl;
            }
            errorCount++;
        }
    }

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);

    static int frameCount = 0;
    frameCount++;
    if (frameCount % 120 == 0) {
        std::cout << "\n=== GLASS BUNNY DEBUG FRAME " << frameCount << " ===" << std::endl;
        std::cout << "Window size: " << width << "x" << height << std::endl;
        std::cout << "Camera pos: (" << camera->Position.x << ", " << camera->Position.y << ", " << camera->Position.z << ")" << std::endl;
        std::cout << "Bunny pos: (" << modelPosition.x << ", " << modelPosition.y << ", " << modelPosition.z << ")" << std::endl;
        std::cout << "Distance to bunny: " << glm::distance(camera->Position, modelPosition) << std::endl;
        std::cout << "Rotations - X:" << debugRotationX << "° Y:" << debugRotationY << "° Z:" << debugRotationZ << "°" << std::endl;
        std::cout << "Scale: " << bunnyScale << std::endl;
        if (model && !model->meshes.empty()) {
            std::cout << "Model: " << model->meshes.size() << " meshes, "
                << model->meshes[0].vertices.size() << " vertices" << std::endl;
        }
        else {
            std::cout << "Model: NOT LOADED - using fallback rendering" << std::endl;
        }
        std::cout << "Wireframe: " << (wireframe ? "ON" : "OFF") << std::endl;
        std::cout << "========================================" << std::endl;
    }
}