#include "Init.hpp"
#include "Menu.hpp"

#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>

Init::Init() : Window() {
    glEnable(GL_DEPTH_TEST);

    camera = std::make_unique<Camera>(glm::vec3(0.0f, 0.0f, 0.5f));
    menu = std::make_unique<Menu>();

    lastX = getWindowWidth() / 2.0f;
    lastY = getWindowHeight() / 2.0f;
    firstClick = true;
    glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Init::initialize() {
    shader = std::make_unique<Shader>("../../../shaders/default.vert", "../../../shaders/default.frag", "../../../shaders/default.geom");
    textRender = std::make_unique<Shader>("../../../shaders/textShader.vert", "../../../shaders/textShader.frag");
    normalsShader = std::make_unique<Shader>("../../../shaders/default.vert", "../../../shaders/normals.frag", "../../../shaders/normals.geom");

    if (!shader || !textRender || !normalsShader) {
        MyglobalLogger().logMessage(Logger::ERROR, "Failed to load shaders!", __FILE__, __LINE__);
        return;
    }

    GLint success;
    GLchar infoLog[512];
    glGetProgramiv(shader->ID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader->ID, 512, NULL, infoLog);
        MyglobalLogger().logMessage(Logger::ERROR, "Shader linking failed: " + std::string(infoLog), __FILE__, __LINE__);
    }
    glGetProgramiv(textRender->ID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(textRender->ID, 512, NULL, infoLog);
        MyglobalLogger().logMessage(Logger::ERROR, "Text shader linking failed: " + std::string(infoLog), __FILE__, __LINE__);
    }
    glGetProgramiv(normalsShader->ID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(normalsShader->ID, 512, NULL, infoLog);
        MyglobalLogger().logMessage(Logger::ERROR, "Normals shader linking failed: " + std::string(infoLog), __FILE__, __LINE__);
    }

    try {
        model = std::make_unique<Model>("../../../models/bunny/scene.gltf");
        MyglobalLogger().logMessage(Logger::INFO, "Successfully loaded GLTF model: scene.gltf", __FILE__, __LINE__);

        glm::vec3 overallMin(FLT_MAX);
        glm::vec3 overallMax(-FLT_MAX);

        if (model && !model->meshes.empty()) {
            for (size_t i = 0; i < model->meshes.size(); ++i) {
                const auto& mesh = model->meshes[i];

                if (!mesh.vertices.empty()) {
                    glm::vec3 minBounds(FLT_MAX);
                    glm::vec3 maxBounds(-FLT_MAX);

                    for (const auto& vertex : mesh.vertices) {
                        minBounds = glm::min(minBounds, vertex.position);
                        maxBounds = glm::max(maxBounds, vertex.position);
                        overallMin = glm::min(overallMin, vertex.position);
                        overallMax = glm::max(overallMax, vertex.position);
                    }

                    glm::vec3 size = maxBounds - minBounds;
                }
            }

            menu->setModelBounds(overallMin, overallMax);
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

    if (!menu->initialize(getWindow())) {
        MyglobalLogger().logMessage(Logger::ERROR, "Failed to initialize menu system!", __FILE__, __LINE__);
        return;
    }

    shader->use();

    glfwSetWindowUserPointer(getWindow(), this);

    glfwSetMouseButtonCallback(getWindow(), [](GLFWwindow* window, int button, int action, int mods) {
        ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
        Init* engine = static_cast<Init*>(glfwGetWindowUserPointer(window));
        engine->mouseButtonCallback(window, button, action, mods);
        });

    glfwSetKeyCallback(getWindow(), [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
        });

    glfwSetCharCallback(getWindow(), [](GLFWwindow* window, unsigned int codepoint) {
        ImGui_ImplGlfw_CharCallback(window, codepoint);
        });

    glfwSetCursorPosCallback(getWindow(), [](GLFWwindow* window, double xpos, double ypos) {
        Init* engine = static_cast<Init*>(glfwGetWindowUserPointer(window));
        engine->cursorPosCallback(window, xpos, ypos);
        });

    glfwSetScrollCallback(getWindow(), [](GLFWwindow* window, double xoffset, double yoffset) {
        Init* engine = static_cast<Init*>(glfwGetWindowUserPointer(window));
        engine->scroll_callback(window, xoffset, yoffset);
        });
}

void Init::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && menu->isEditorModeActive()) {
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureMouse) {
            return;
        }

        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        int width, height;
        glfwGetWindowSize(window, &width, &height);


        float ndcX = (2.0f * mouseX) / width - 1.0f;
        float ndcY = 1.0f - (2.0f * mouseY) / height;

        glm::vec4 clipCoords = glm::vec4(ndcX, ndcY, -1.0f, 1.0f);
        glm::vec4 eyeCoords = glm::inverse(projection) * clipCoords;
        eyeCoords = glm::vec4(eyeCoords.x, eyeCoords.y, -1.0f, 0.0f);

        glm::vec3 worldRay = glm::normalize(glm::vec3(glm::inverse(view) * eyeCoords));
        glm::vec3 rayOrigin = camera->Position;

        glm::vec3 modelPos = menu->getModelPosition();
        glm::vec3 modelScale = menu->getModelScale();
        glm::vec3 boxMin = menu->localMinBounds * modelScale + modelPos;
        glm::vec3 boxMax = menu->localMaxBounds * modelScale + modelPos;


        if (rayAABBIntersectWorld(rayOrigin, worldRay, boxMin, boxMax)) {
            menu->setModelSelected(true);
        }
        else {
            menu->setModelSelected(false);

            glm::vec3 expandedMin = boxMin - glm::vec3(2.0f);
            glm::vec3 expandedMax = boxMax + glm::vec3(2.0f);

            if (rayAABBIntersectWorld(rayOrigin, worldRay, expandedMin, expandedMax)) {
                menu->setModelSelected(true);
            }
        }
    }
}

bool Init::rayAABBIntersectWorld(const glm::vec3& origin, const glm::vec3& dir, const glm::vec3& boxMin, const glm::vec3& boxMax) {
    glm::vec3 invDir;
    invDir.x = (abs(dir.x) > 0.0001f) ? 1.0f / dir.x : 1e30f;
    invDir.y = (abs(dir.y) > 0.0001f) ? 1.0f / dir.y : 1e30f;
    invDir.z = (abs(dir.z) > 0.0001f) ? 1.0f / dir.z : 1e30f;

    glm::vec3 t1 = (boxMin - origin) * invDir;
    glm::vec3 t2 = (boxMax - origin) * invDir;

    glm::vec3 tmin = glm::min(t1, t2);
    glm::vec3 tmax = glm::max(t1, t2);

    float t_near = glm::max(glm::max(tmin.x, tmin.y), tmin.z);
    float t_far = glm::min(glm::min(tmax.x, tmax.y), tmax.z);

    bool intersects = t_near <= t_far && t_far >= 0.0f;

    return intersects;
}

void Init::processInput(GLFWwindow* window) {
    static float lastFrame = 0.0f;
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    menu->handleEditorToggle(window);

    if (menu->isEditorModeActive()) {
        menu->handleInput(window);
    }

    if (!menu->isEditorModeActive()) {
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
    }

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

void Init::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);

    if (menu->isEditorModeActive()) {
        firstClick = true;
    }
    else {
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
}

void Init::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);

    if (!menu->isEditorModeActive()) {
        camera->ProcessMouseScroll(yoffset);
    }
}

void Init::render() {
    glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int width, height;
    glfwGetWindowSize(getWindow(), &width, &height);
    glViewport(0, 0, width, height);

    projection = glm::perspective(glm::radians(camera->Zoom), (float)width / (float)height, 0.1f, 100.0f);
    view = camera->getViewMatrix();

    menu->render(view, projection);

    glm::mat4 modelMatrix = menu->getModelMatrix();
    glm::vec3 modelPosition = menu->getModelPosition();
    glm::vec3 modelRotation = menu->getModelRotation();
    glm::vec3 modelScale = menu->getModelScale();

    bool wireframe = menu->isWireframeMode();
    bool showNormals = menu->isShowNormals();
    bool geometryEffects = menu->isGeometryEffects();

    if (!menu->isEditorModeActive()) {
        static bool tKeyPressed = false;
        bool tKeyCurrentlyPressed = (glfwGetKey(getWindow(), GLFW_KEY_T) == GLFW_PRESS);
        if (tKeyCurrentlyPressed && !tKeyPressed) {
            wireframe = !wireframe;
            menu->setWireframeMode(wireframe);
        }
        tKeyPressed = tKeyCurrentlyPressed;

        static bool nKeyPressed = false;
        bool nKeyCurrentlyPressed = (glfwGetKey(getWindow(), GLFW_KEY_N) == GLFW_PRESS);
        if (nKeyCurrentlyPressed && !nKeyPressed) {
            showNormals = !showNormals;
            menu->setShowNormals(showNormals);
        }
        nKeyPressed = nKeyCurrentlyPressed;

        static bool gKeyPressed = false;
        bool gKeyCurrentlyPressed = (glfwGetKey(getWindow(), GLFW_KEY_G) == GLFW_PRESS);
        if (gKeyCurrentlyPressed && !gKeyPressed) {
            geometryEffects = !geometryEffects;
            menu->setGeometryEffects(geometryEffects);
        }
        gKeyPressed = gKeyCurrentlyPressed;
    }

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
            shader->setMat4("view", view);
            shader->setMat4("projection", projection);
            shader->setVec3("camPos", camera->Position);

            GLint timeLocation = glGetUniformLocation(shader->ID, "time");
            if (timeLocation != -1) {
                shader->setFloat("time", (float)glfwGetTime());
            }
            else {
                MyglobalLogger().logMessage(Logger::WARNING, "Failed to find uniform: time", __FILE__, __LINE__);
            }

            model->Draw(*shader, *camera, modelMatrix);

            if (showNormals && normalsShader) {
                glDisable(GL_BLEND);
                glDepthMask(GL_TRUE);

                normalsShader->use();
                normalsShader->setMat4("view", view);
                normalsShader->setMat4("projection", projection);
                normalsShader->setVec3("camPos", camera->Position);

                GLint normalsTimeLocation = glGetUniformLocation(normalsShader->ID, "time");
                if (normalsTimeLocation != -1) {
                    normalsShader->setFloat("time", (float)glfwGetTime());
                }
                else {
                    MyglobalLogger().logMessage(Logger::WARNING, "Failed to find uniform: time in normalsShader", __FILE__, __LINE__);
                }

                model->Draw(*normalsShader, *camera, modelMatrix);

                if (!wireframe) {
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    glDepthMask(GL_FALSE);
                }
            }
        }
        catch (const std::exception& e) {
            MyglobalLogger().logMessage(Logger::ERROR, "Problem with initiaize" + std::string(e.what()), __FILE__, __LINE__);
        }

        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            MyglobalLogger().logMessage(Logger::ERROR, "OpenGL error after model draw: " + error, __FILE__, __LINE__);
        }
    }

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);

    if (font) {
        try {
            textRender->use();
            glm::mat4 textProjection = glm::ortho(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, -1.0f, 1.0f);
            textRender->setMat4("projection", textProjection);
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

            std::string FPSstring = "FPS: " + std::to_string((int)(1.0f / deltaTime));
            font->print(FPSstring.c_str(), 10.0f, 85.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));

            if (model && !model->meshes.empty()) {
                textRender->setVec3("Color", glm::vec3(0.0f, 0.8f, 1.0f));
                std::string meshInfo = "Glass Bunny | Meshes: " + std::to_string(model->meshes.size()) +
                    " | Vertices: " + std::to_string(model->meshes[0].vertices.size());
                font->print(meshInfo.c_str(), 10.0f, 130.0f, 1.0f, glm::vec3(0.0f, 0.8f, 1.0f));

                textRender->setVec3("Color", glm::vec3(0.8f, 1.0f, 0.8f));
                std::string glassInfo = "GLASS EFFECT ACTIVE";
                font->print(glassInfo.c_str(), 10.0f, 105.0f, 1.0f, glm::vec3(0.8f, 1.0f, 0.8f));

                std::string rotInfo = "Rotations X:" + std::to_string((int)modelRotation.x) +
                    " Y:" + std::to_string((int)modelRotation.y) +
                    " Z:" + std::to_string((int)modelRotation.z) +
                    " Scale:" + std::to_string(modelScale.x);
                font->print(rotInfo.c_str(), 10.0f, 155.0f, 0.8f, glm::vec3(1.0f, 1.0f, 0.0f));
            }
            else {
                textRender->setVec3("Color", glm::vec3(1.0f, 0.0f, 0.0f));
                std::string noModelText = "No model loaded - showing fallback points";
                font->print(noModelText.c_str(), 10.0f, 105.0f, 0.7f, glm::vec3(1.0f, 0.0f, 0.0f));
            }

            float statusY = 180.0f;
            if (wireframe) {
                textRender->setVec3("Color", glm::vec3(1.0f, 0.5f, 0.0f));
                std::string wireframeText = "WIREFRAME MODE ON";
                font->print(wireframeText.c_str(), 10.0f, statusY, 1.0f, glm::vec3(1.0f, 0.5f, 0.0f));
                statusY += 25.0f;
            }

            if (showNormals) {
                textRender->setVec3("Color", glm::vec3(0.8f, 0.3f, 0.0f));
                std::string normalsText = "NORMALS DISPLAY ON";
                font->print(normalsText.c_str(), 10.0f, statusY, 1.0f, glm::vec3(0.8f, 0.3f, 0.0f));
                statusY += 25.0f;
            }

            if (geometryEffects) {
                textRender->setVec3("Color", glm::vec3(1.0f, 0.0f, 1.0f));
                std::string geomText = "GEOMETRY EFFECTS ON";
                font->print(geomText.c_str(), 10.0f, statusY, 1.0f, glm::vec3(1.0f, 0.0f, 1.0f));
                statusY += 25.0f;
            }

            if (menu->isEditorModeActive()) {
                textRender->setVec3("Color", glm::vec3(0.2f, 1.0f, 0.2f));
                std::string editorText = "EDITOR MODE ACTIVE - Press B to exit";
                font->print(editorText.c_str(), 10.0f, statusY, 1.0f, glm::vec3(0.2f, 1.0f, 0.2f));
                statusY += 25.0f;

                textRender->setVec3("Color", glm::vec3(1.0f, 1.0f, 0.0f));
                std::string guizmoText = "Guizmo: ";
                if (menu->modelSelected) {
                    if (ImGuizmo::IsUsing()) guizmoText += "TRANSFORMING";
                    else if (ImGuizmo::IsOver()) guizmoText += "HOVER";
                    else guizmoText += "READY";
                }
                else {
                    guizmoText += "WAITING (click model)";
                }
                font->print(guizmoText.c_str(), 10.0f, statusY, 0.8f, glm::vec3(1.0f, 1.0f, 0.0f));
                statusY += 25.0f;
            }

            textRender->setVec3("Color", glm::vec3(0.7f, 0.7f, 0.7f));
            std::string controlsText = "Controls: WASD+Mouse | Space/Shift-Up/Down | T-Wireframe | N-Normals | G-GeomFX | B-Editor | ESC-Exit";
            font->print(controlsText.c_str(), 10.0f, static_cast<float>(height - 30), 0.5f, glm::vec3(0.7f, 0.7f, 0.7f));

            std::string debugInstructions = "EDITOR: B-Toggle Mode | 1/2/3-Transform Modes | Drag Gizmo to transform";
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
}