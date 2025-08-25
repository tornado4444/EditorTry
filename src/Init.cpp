#include "Init.hpp"
#include "Menu.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

Init::Init() : Window() {
    glEnable(GL_DEPTH_TEST);
    camera = std::make_unique<Camera>(glm::vec3(0.0f, 0.0f, 5.0f));
    menu = std::make_unique<Menu>();
    bvh = std::make_unique<BVH>();
    lastX = getWindowWidth() / 2.0f;
    lastY = getWindowHeight() / 2.0f;
    firstClick = true;
    glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Init::initialize() {
    shader = std::make_unique<Shader>("../shaders/default.vert", "../shaders/default.frag", "../shaders/default.geom");
    textRender = std::make_unique<Shader>("../shaders/textShader.vert", "../shaders/textShader.frag");
    normalsShader = std::make_unique<Shader>("../shaders/default.vert", "../shaders/normals.frag", "../shaders/normals.geom");
    aabbShader = std::make_unique<Shader>("../shaders/aabb.vert", "../shaders/aabb.frag");

    mortonShader = std::make_unique<Shader>("../shaders/lbvh_morton_codes.comp");
    sortShader = std::make_unique<Shader>("../shaders/lbvh_single_radixsort.comp");
    hierarchyShader = std::make_unique<Shader>("../shaders/lbvh_hierarchy.comp");
    lbvhAABBShader = std::make_unique<Shader>("../shaders/lbvh_bounding_boxes.comp");


    if (!shader || !textRender || !normalsShader || !aabbShader || !mortonShader || !sortShader || !hierarchyShader || !lbvhAABBShader) {
        MyglobalLogger().logMessage(Logger::ERROR, "Failed to load shaders!", __FILE__, __LINE__);
        return;
    }

    GLint success;
    GLchar infoLog[512];
    for (const auto& s : { shader.get(), textRender.get(), normalsShader.get(), aabbShader.get(), mortonShader.get(), sortShader.get(), hierarchyShader.get(), lbvhAABBShader.get() }) {
        glGetProgramiv(s->ID, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(s->ID, 512, NULL, infoLog);
            MyglobalLogger().logMessage(Logger::ERROR, "Shader linking failed: " + std::string(infoLog), __FILE__, __LINE__);
        }
    }

    MyglobalLogger().logMessage(Logger::INFO, "OpenGL Version: " + std::string(reinterpret_cast<const char*>(glGetString(GL_VERSION))), __FILE__, __LINE__);

    try {
        model = std::make_unique<Model>("models/bunny/scene.gltf");
        MyglobalLogger().logMessage(Logger::INFO, "Successfully loaded GLTF model: scene.gltf", __FILE__, __LINE__);

        glm::vec3 overallMin(FLT_MAX);
        glm::vec3 overallMax(-FLT_MAX);
        std::vector<glm::vec3> positions;
        std::vector<uint32_t> indices;

        if (model && !model->meshes.empty()) {
            for (size_t i = 0; i < model->meshes.size(); ++i) {
                const auto& mesh = model->meshes[i];
                if (!mesh.vertices.empty()) {
                    for (const auto& vertex : mesh.vertices) {
                        positions.push_back(vertex.position);
                        overallMin = glm::min(overallMin, vertex.position);
                        overallMax = glm::max(overallMax, vertex.position);
                    }
                    for (const auto& index : mesh.indices) {
                        indices.push_back(index);
                    }
                }
            }
            menu->setModelBounds(overallMin, overallMax);

            glm::mat4 initialModelMatrix = menu->getModelMatrix();

            double startTime = glfwGetTime();
            bvh->buildLBVHDynamic(positions, indices, mortonShader.get(), sortShader.get(), hierarchyShader.get(), lbvhAABBShader.get());
            menu->lastLBVHBuildTime = (glfwGetTime() - startTime) * 1000.0f;
        }
    }
    catch (const std::exception& e) {
        MyglobalLogger().logMessage(Logger::ERROR, "Failed to load GLTF model: " + std::string(e.what()), __FILE__, __LINE__);
        return;
    }

    std::vector<GLfloat> cubeVertices = {
        -0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f
    };
    std::vector<GLuint> cubeIndices = {
        0, 1, 1, 2, 2, 3, 3, 0,
        4, 5, 5, 6, 6, 7, 7, 4,
        0, 4, 1, 5, 2, 6, 3, 7
    };
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glGenBuffers(1, &cubeEBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, cubeVertices.size() * sizeof(GLfloat), cubeVertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cubeIndices.size() * sizeof(GLuint), cubeIndices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        MyglobalLogger().logMessage(Logger::ERROR, "OpenGL error after AABB geometry setup: " + std::to_string(error), __FILE__, __LINE__);
    }

    font = std::make_unique<Font>("../Fonts/comicSans_32.fnt");
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
        if (io.WantCaptureMouse) return;

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
            glm::vec3 expandedMin = boxMin - glm::vec3(2.0f);
            glm::vec3 expandedMax = boxMax + glm::vec3(2.0f);
            if (rayAABBIntersectWorld(rayOrigin, worldRay, expandedMin, expandedMax)) {
                menu->setModelSelected(true);
            }
            else {
                menu->setModelSelected(false);
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
    return t_near <= t_far && t_far >= 0.0f;
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

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);

    int width, height;
    glfwGetWindowSize(getWindow(), &width, &height);
    glViewport(0, 0, width, height);

    projection = glm::perspective(glm::radians(camera->Zoom), (float)width / (float)height, 0.1f, 200.0f);
    view = camera->getViewMatrix();

    static glm::mat4 lastModelMatrix = glm::mat4(0.0f);
    glm::mat4 currentModelMatrix = menu->getModelMatrix();

    if (lastModelMatrix != currentModelMatrix || menu->rebuildLBVH) {
        if (model && !model->meshes.empty()) {
            std::vector<glm::vec3> positions;
            std::vector<uint32_t> indices;

            for (const auto& mesh : model->meshes) {
                for (const auto& vertex : mesh.vertices) {
                    positions.push_back(vertex.position); 
                }
                for (const auto& index : mesh.indices) {
                    indices.push_back(index);
                }
            }

            std::vector<glm::vec3> transformedPositions;
            transformedPositions.reserve(positions.size());
            for (const auto& pos : positions) {
                transformedPositions.push_back(glm::vec3(currentModelMatrix * glm::vec4(pos, 1.0f)));
            }

            double startTime = glfwGetTime();
            bvh->buildLBVHDynamic(transformedPositions, indices, mortonShader.get(), sortShader.get(), hierarchyShader.get(), lbvhAABBShader.get());
            menu->lastLBVHBuildTime = (glfwGetTime() - startTime) * 1000.0f;
            menu->rebuildLBVH = false;
            lastModelMatrix = currentModelMatrix;

            MyglobalLogger().logMessage(Logger::INFO, "LBVH rebuilt with " + std::to_string(bvh->numInternalNodes) + " nodes (transform changed)", __FILE__, __LINE__);
        }
    }

    menu->render(view, projection);

    glm::mat4 modelMatrix = menu->getModelMatrix();
    glm::vec3 modelPosition = menu->getModelPosition();
    glm::vec3 modelRotation = menu->getModelRotation();
    glm::vec3 modelScale = menu->getModelScale();

    bool wireframe = menu->isWireframeMode();
    bool showNormals = menu->isShowNormals();
    bool geometryEffects = menu->isGeometryEffects();
    bool showLBVH = menu->showLBVH;

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

        static bool lKeyPressed = false;
        bool lKeyCurrentlyPressed = (glfwGetKey(getWindow(), GLFW_KEY_L) == GLFW_PRESS);
        if (lKeyCurrentlyPressed && !lKeyPressed) {
            showLBVH = !showLBVH;
            menu->showLBVH = showLBVH;
        }
        lKeyPressed = lKeyCurrentlyPressed;
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
                model->Draw(*normalsShader, *camera, modelMatrix);
                if (!wireframe) {
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    glDepthMask(GL_FALSE);
                }
            }

            if (showLBVH && bvh->numInternalNodes > 0) {
                MyglobalLogger().logMessage(Logger::DEBUG, "Rendering LBVH: " + std::to_string(bvh->numInternalNodes) + " nodes", __FILE__, __LINE__);

                aabbShader->use();
                aabbShader->setMat4("view", view);
                aabbShader->setMat4("projection", projection);

                glBindVertexArray(cubeVAO);
                glBindBuffer(GL_ARRAY_BUFFER, bvh->aabbInstanceVBO);

                glEnableVertexAttribArray(1);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3), (void*)0);
                glVertexAttribDivisor(1, 1);

                glEnableVertexAttribArray(2);
                glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3), (void*)sizeof(glm::vec3));
                glVertexAttribDivisor(2, 1);

                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                glLineWidth(2.0f);
                glDisable(GL_DEPTH_TEST);

                glDrawElementsInstanced(GL_LINES, 24, GL_UNSIGNED_INT, 0, bvh->numInternalNodes);

                glEnable(GL_DEPTH_TEST);
                glDisableVertexAttribArray(1);
                glDisableVertexAttribArray(2);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindVertexArray(0);

                if (!wireframe) {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                }

                GLenum error = glGetError();
                if (error != GL_NO_ERROR) {
                    MyglobalLogger().logMessage(Logger::ERROR, "OpenGL error after AABB draw: " + std::to_string(error), __FILE__, __LINE__);
                }
            }
        }
        catch (const std::exception& e) {
            MyglobalLogger().logMessage(Logger::ERROR, "Problem with rendering: " + std::string(e.what()), __FILE__, __LINE__);
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

                if (bvh->numInternalNodes > 0) {
                    textRender->setVec3("Color", glm::vec3(1.0f, 0.0f, 1.0f));
                    std::string lbvhDebug = "LBVH Nodes: " + std::to_string(bvh->numInternalNodes) +
                        " VBO: " + std::to_string(bvh->aabbInstanceVBO);
                    font->print(lbvhDebug.c_str(), 10.0f, 235.0f, 0.8f, glm::vec3(1.0f, 0.0f, 1.0f));
                }
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
            if (showLBVH) {
                textRender->setVec3("Color", glm::vec3(0.0f, 1.0f, 1.0f));
                std::string lbvhText = "LBVH VISUALIZATION ON";
                font->print(lbvhText.c_str(), 10.0f, statusY, 1.0f, glm::vec3(0.0f, 1.0f, 1.0f));
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
            std::string controlsText = "Controls: WASD+Mouse | Space/Shift-Up/Down | T-Wireframe | N-Normals | G-GeomFX | L-LBVH | B-Editor | ESC-Exit";
            font->print(controlsText.c_str(), 10.0f, static_cast<float>(height - 30), 0.5f, glm::vec3(0.7f, 0.7f, 0.7f));

            std::string debugInstructions = "EDITOR: B-Toggle Mode | 1/2/3-Transform Modes | L-LBVH | Drag Gizmo to transform";
            font->print(debugInstructions.c_str(), 10.0f, static_cast<float>(height - 50), 0.5f, glm::vec3(0.0f, 1.0f, 1.0f));
        }
        catch (const std::exception& e) {
            static int errorCount = 0;
            if (errorCount % 300 == 0) {
                MyglobalLogger().logMessage(Logger::ERROR, "Font rendering error: " + std::string(e.what()), __FILE__, __LINE__);
            }
            errorCount++;
        }
    }

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
}
