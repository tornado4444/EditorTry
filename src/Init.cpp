#include "Init.hpp"
#include "Menu.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <algorithm>
#include <cfloat>
#include <filesystem>
#include <array>
#include "stb_image.hpp"

namespace {
    constexpr float kEarthRadius = 6378000.0f;

    GLint findUniform(GLuint program, const char* name) {
        return glGetUniformLocation(program, name);
    }

    void setFloatIfPresent(GLuint program, const char* name, float value) {
        const GLint location = findUniform(program, name);
        if (location != -1) {
            glUniform1f(location, value);
        }
    }

    void setVec3IfPresent(GLuint program, const char* name, const glm::vec3& value) {
        const GLint location = findUniform(program, name);
        if (location != -1) {
            glUniform3f(location, value.x, value.y, value.z);
        }
    }

    void setIntIfPresent(GLuint program, const char* name, int value) {
        const GLint location = findUniform(program, name);
        if (location != -1) {
            glUniform1i(location, value);
        }
    }

    GLuint createSolidTexture2D(const std::array<unsigned char, 4>& rgba) {
        GLuint textureId = 0;
        glGenTextures(1, &textureId);
        if (textureId == 0) {
            return 0;
        }

        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());
        glBindTexture(GL_TEXTURE_2D, 0);

        return textureId;
    }
}

Init::Init() : Window() {
    glEnable(GL_DEPTH_TEST);
    camera = std::make_unique<Camera>(
        glm::vec3(0.0f, 3.5f, 7.5f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        Camera::YAW_DEFAULT,
        -12.0f
    );
    menu = std::make_unique<Menu>();
    bvh = std::make_unique<BVH>();
    lastX = getWindowWidth() / 2.0f;
    lastY = getWindowHeight() / 2.0f;
    firstClick = true;
    glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

Init::~Init() {
    destroyEnvironmentResources();
}

std::filesystem::path Init::resolveResourcePath(const std::string& relativePath) {
    namespace fs = std::filesystem;

    fs::path direct(relativePath);
    if (fs::exists(direct)) {
        return direct;
    }

    fs::path cursor = fs::current_path();
    for (int i = 0; i < 8; ++i) {
        fs::path candidate = cursor / relativePath;
        if (fs::exists(candidate)) {
            return candidate;
        }
        if (!cursor.has_parent_path()) {
            break;
        }
        cursor = cursor.parent_path();
    }

    cursor = fs::current_path();
    const fs::path fileName = fs::path(relativePath).filename();
    for (int i = 0; i < 8; ++i) {
        fs::path texturesCandidate = cursor / "textures" / fileName;
        if (fs::exists(texturesCandidate)) {
            return texturesCandidate;
        }
        if (!cursor.has_parent_path()) {
            break;
        }
        cursor = cursor.parent_path();
    }

    return direct;
}

GLuint Init::loadTexture2DForAtmosphere(const std::filesystem::path& path) {
    stbi_set_flip_vertically_on_load(false);

    int width = 0;
    int height = 0;
    int channels = 0;
    unsigned char* data = stbi_load(path.string().c_str(), &width, &height, &channels, 4);
    if (!data) {
        MyglobalLogger().logMessage(Logger::ERROR, "Failed to load atmosphere 2D texture: " + path.string(), __FILE__, __LINE__);
        return 0;
    }

    GLuint textureId = 0;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(data);
    return textureId;
}

GLuint Init::loadTexture3DFromAtlas(const std::filesystem::path& path) {
    stbi_set_flip_vertically_on_load(false);

    int width = 0;
    int height = 0;
    int channels = 0;
    unsigned char* data = stbi_load(path.string().c_str(), &width, &height, &channels, 4);
    if (!data) {
        MyglobalLogger().logMessage(Logger::ERROR, "Failed to load atmosphere 3D texture atlas: " + path.string(), __FILE__, __LINE__);
        return 0;
    }

    const long long expectedHeight = 1LL * width * width;
    if (height != expectedHeight) {
        stbi_image_free(data);
        MyglobalLogger().logMessage(
            Logger::ERROR,
            "Invalid 3D atlas dimensions for " + path.string() + ". Expected height: " + std::to_string(expectedHeight) +
            ", actual height: " + std::to_string(height),
            __FILE__,
            __LINE__
        );
        return 0;
    }

    GLuint textureId = 0;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_3D, textureId);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, width, width, width, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_3D, 0);

    stbi_image_free(data);
    return textureId;
}

bool Init::initializeEnvironmentResources() {
    try {
        environmentShader = std::make_unique<Shader>("../../../shaders/environment.vert", "../../../shaders/environment.frag");
    }
    catch (const std::exception& e) {
        MyglobalLogger().logMessage(Logger::ERROR, "Failed to initialize environment shader: " + std::string(e.what()), __FILE__, __LINE__);
        atmosphereReady = false;
        return false;
    }

    const float fullscreenQuadVertices[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f,  1.0f
    };

    glGenVertexArrays(1, &atmosphereVAO);
    glGenBuffers(1, &atmosphereVBO);
    glBindVertexArray(atmosphereVAO);
    glBindBuffer(GL_ARRAY_BUFFER, atmosphereVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(fullscreenQuadVertices), fullscreenQuadVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    lowFrequencyTex3D = loadTexture3DFromAtlas(resolveResourcePath("../../../textures/LowFrequency3DTexture.tga"));
    highFrequencyTex3D = loadTexture3DFromAtlas(resolveResourcePath("../../../textures/HighFrequency3DTexture.tga"));
    weatherTex2D = loadTexture2DForAtmosphere(resolveResourcePath("../../../textures/weathermap.png"));
    curlNoiseTex2D = loadTexture2DForAtmosphere(resolveResourcePath("../../../textures/curlNoise.png"));

    auto loadOptionalTexture2D = [this](const std::vector<std::string>& candidates, const std::array<unsigned char, 4>& fallbackColor, bool& loadedFromFile, const std::string& label) -> GLuint {
        loadedFromFile = false;
        for (const auto& relativePath : candidates) {
            const std::filesystem::path resolved = resolveResourcePath(relativePath);
            if (!std::filesystem::exists(resolved)) {
                continue;
            }

            GLuint textureId = loadTexture2DForAtmosphere(resolved);
            if (textureId != 0) {
                loadedFromFile = true;
                MyglobalLogger().logMessage(Logger::INFO, "Loaded " + label + " texture: " + resolved.string(), __FILE__, __LINE__);
                return textureId;
            }
        }

        GLuint fallback = createSolidTexture2D(fallbackColor);
        if (fallback != 0) {
            MyglobalLogger().logMessage(Logger::WARNING, "Missing " + label + " texture files. Using fallback 1x1 texture.", __FILE__, __LINE__);
        } else {
            MyglobalLogger().logMessage(Logger::ERROR, "Failed to create fallback " + label + " texture.", __FILE__, __LINE__);
        }
        return fallback;
    };

    moonTex2D = loadOptionalTexture2D(
        {
            "../../../textures/nasa/NASA-LRO-Moon-mosaic.png",
            "../../../textures/nasa/moon_mosaic.png",
            "../../../textures/NASA-LRO-Moon-mosaic.png",
            "../../../textures/moon_mosaic.png"
        },
        { { 186, 186, 186, 255 } },
        hasMoonTexture,
        "moon"
    );

    starTex2D = loadOptionalTexture2D(
        {
            "../../../textures/nasa/PIA15482.jpg",
            "../../../textures/nasa/wise_sky_rectangular.jpg",
            "../../../textures/PIA15482.jpg",
            "../../../textures/wise_sky_rectangular.jpg"
        },
        { { 3, 7, 14, 255 } },
        hasStarTexture,
        "star map"
    );

    atmosphereReady = environmentShader &&
        lowFrequencyTex3D != 0 &&
        highFrequencyTex3D != 0 &&
        weatherTex2D != 0 &&
        curlNoiseTex2D != 0 &&
        moonTex2D != 0 &&
        starTex2D != 0;
    if (!atmosphereReady) {
        MyglobalLogger().logMessage(Logger::ERROR, "Atmosphere resources are incomplete; ocean/cloud pass disabled.", __FILE__, __LINE__);
        destroyEnvironmentResources();
        return false;
    }

    MyglobalLogger().logMessage(Logger::INFO, "Atmosphere resources loaded: unified ocean + volumetric clouds mode enabled.", __FILE__, __LINE__);
    return true;
}

void Init::destroyEnvironmentResources() {
    if (lowFrequencyTex3D != 0) {
        glDeleteTextures(1, &lowFrequencyTex3D);
        lowFrequencyTex3D = 0;
    }
    if (highFrequencyTex3D != 0) {
        glDeleteTextures(1, &highFrequencyTex3D);
        highFrequencyTex3D = 0;
    }
    if (weatherTex2D != 0) {
        glDeleteTextures(1, &weatherTex2D);
        weatherTex2D = 0;
    }
    if (curlNoiseTex2D != 0) {
        glDeleteTextures(1, &curlNoiseTex2D);
        curlNoiseTex2D = 0;
    }
    if (moonTex2D != 0) {
        glDeleteTextures(1, &moonTex2D);
        moonTex2D = 0;
    }
    if (starTex2D != 0) {
        glDeleteTextures(1, &starTex2D);
        starTex2D = 0;
    }
    if (atmosphereVBO != 0) {
        glDeleteBuffers(1, &atmosphereVBO);
        atmosphereVBO = 0;
    }
    if (atmosphereVAO != 0) {
        glDeleteVertexArrays(1, &atmosphereVAO);
        atmosphereVAO = 0;
    }

    atmosphereReady = false;
    hasMoonTexture = false;
    hasStarTexture = false;
}

void Init::renderEnvironment(int width, int height, float timeSeconds) {
    if (!atmosphereReady || !environmentShader) {
        return;
    }

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glViewport(0, 0, width, height);

    environmentShader->use();
    setFloatIfPresent(environmentShader->ID, "Time", timeSeconds);
    setFloatIfPresent(environmentShader->ID, "screenWidth", static_cast<float>(width));
    setFloatIfPresent(environmentShader->ID, "screenHeight", static_cast<float>(height));
    setVec3IfPresent(environmentShader->ID, "cameraPosition", camera->Position);
    setVec3IfPresent(environmentShader->ID, "cameraFront", camera->Front);
    setVec3IfPresent(environmentShader->ID, "cameraUp", camera->Up);
    setVec3IfPresent(environmentShader->ID, "cameraRight", camera->Right);

    const glm::vec3 earthCenter(camera->Position.x, -kEarthRadius, camera->Position.z);
    setVec3IfPresent(environmentShader->ID, "EarthCenter", earthCenter);

    const float cloudCoverage = std::max(0.0f, menu->getCloudCoverage());
    const float cloudDensity = std::max(0.0f, menu->getCloudDensity() * cloudCoverage);
    setFloatIfPresent(environmentShader->ID, "CloudBottom", 1300.0f);
    setFloatIfPresent(environmentShader->ID, "CloudTop", 7600.0f);
    setFloatIfPresent(environmentShader->ID, "CloudDensity", cloudDensity);
    setFloatIfPresent(environmentShader->ID, "CloudCoverage", cloudCoverage);
    setFloatIfPresent(environmentShader->ID, "CloudSoftness", menu->getCloudSoftness());
    setFloatIfPresent(environmentShader->ID, "CloudStorminess", menu->getCloudStorminess());
    setFloatIfPresent(environmentShader->ID, "OceanWaveStrength", menu->getOceanWaveStrength());
    setFloatIfPresent(environmentShader->ID, "UnderwaterDensity", menu->getUnderwaterDensity());
    setFloatIfPresent(environmentShader->ID, "SkyTimeHours", menu->getSkyTimeHours());
    setFloatIfPresent(environmentShader->ID, "StarBrightness", menu->getStarBrightness());
    setFloatIfPresent(environmentShader->ID, "MoonBrightness", menu->getMoonBrightness());
    setFloatIfPresent(environmentShader->ID, "MoonSizeDegrees", menu->getMoonSizeDegrees());
    setFloatIfPresent(environmentShader->ID, "OceanWireframe", menu->isWireframeMode() ? 1.0f : 0.0f);
    setIntIfPresent(environmentShader->ID, "UseMoonTexture", hasMoonTexture ? 1 : 0);
    setIntIfPresent(environmentShader->ID, "UseStarTexture", hasStarTexture ? 1 : 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, lowFrequencyTex3D);
    setIntIfPresent(environmentShader->ID, "lowFrequencyTexture", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, highFrequencyTex3D);
    setIntIfPresent(environmentShader->ID, "highFrequencyTexture", 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, weatherTex2D);
    setIntIfPresent(environmentShader->ID, "WeatherTexture", 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, curlNoiseTex2D);
    setIntIfPresent(environmentShader->ID, "CurlNoiseTexture", 3);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, moonTex2D);
    setIntIfPresent(environmentShader->ID, "MoonTexture", 4);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, starTex2D);
    setIntIfPresent(environmentShader->ID, "StarTexture", 5);

    glBindVertexArray(atmosphereVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, 0);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
}

void Init::initialize() {
    shader = std::make_unique<Shader>("../../../shaders/default.vert", "../../../shaders/default.frag", "../../../shaders/default.geom");
    textRender = std::make_unique<Shader>("../../../shaders/textShader.vert", "../../../shaders/textShader.frag");
    normalsShader = std::make_unique<Shader>("../../../shaders/default.vert", "../../../shaders/normals.frag", "../../../shaders/normals.geom");
    aabbShader = std::make_unique<Shader>("../../../shaders/aabb.vert", "../../../shaders/aabb.frag");

    mortonShader = std::make_unique<Shader>("../../../shaders/lbvh_morton_codes.comp");
    sortShader = std::make_unique<Shader>("../../../shaders/lbvh_single_radixsort.comp");
    hierarchyShader = std::make_unique<Shader>("../../../shaders/lbvh_hierarchy.comp");
    lbvhAABBShader = std::make_unique<Shader>("../../../shaders/lbvh_bounding_boxes.comp");

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

    // Проверка версии OpenGL
    MyglobalLogger().logMessage(Logger::INFO, "OpenGL Version: " + std::string(reinterpret_cast<const char*>(glGetString(GL_VERSION))), __FILE__, __LINE__);

    try {
        model = std::make_unique<Model>("../../../models/bunny/scene.gltf");
        MyglobalLogger().logMessage(Logger::INFO, "Successfully loaded GLTF model: scene.gltf", __FILE__, __LINE__);

        glm::vec3 overallMin(FLT_MAX);
        glm::vec3 overallMax(-FLT_MAX);
        std::vector<glm::vec3> positions;
        std::vector<uint32_t> indices;
        const auto& meshLocalMatrices = model->getMeshLocalMatrices();

        if (model && !model->meshes.empty()) {
            for (size_t i = 0; i < model->meshes.size(); ++i) {
                const auto& mesh = model->meshes[i];
                if (!mesh.vertices.empty()) {
                    const glm::mat4 meshLocal = (i < meshLocalMatrices.size()) ? meshLocalMatrices[i] : glm::mat4(1.0f);
                    const uint32_t vertexOffset = static_cast<uint32_t>(positions.size());
                    for (const auto& vertex : mesh.vertices) {
                        const glm::vec3 modelSpacePos = glm::vec3(meshLocal * glm::vec4(vertex.position, 1.0f));
                        positions.push_back(modelSpacePos);
                        overallMin = glm::min(overallMin, modelSpacePos);
                        overallMax = glm::max(overallMax, modelSpacePos);
                    }
                    for (const auto& index : mesh.indices) {
                        indices.push_back(vertexOffset + index);
                    }
                }
            }
            menu->setModelBounds(overallMin, overallMax);

            glm::mat4 initialModelMatrix = menu->getModelMatrix();
            std::vector<glm::vec3> transformedPositions;
            transformedPositions.reserve(positions.size());
            for (const auto& pos : positions) {
                transformedPositions.push_back(glm::vec3(initialModelMatrix * glm::vec4(pos, 1.0f)));
            }

            double startTime = glfwGetTime();
            bvh->buildLBVHDynamic(transformedPositions, indices, mortonShader.get(), sortShader.get(), hierarchyShader.get(), lbvhAABBShader.get());
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

    font = std::make_unique<Font>("../../../Fonts/comicSans_32.fnt");
    if (!font) {
        MyglobalLogger().logMessage(Logger::ERROR, "Failed to initialize font!", __FILE__, __LINE__);
        return;
    }

    if (!menu->initialize(getWindow())) {
        MyglobalLogger().logMessage(Logger::ERROR, "Failed to initialize menu system!", __FILE__, __LINE__);
        return;
    }

    initializeEnvironmentResources();

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

    menu->handleGlobalInput(window);
    menu->handleEditorToggle(window);
    if (menu->isEditorModeActive()) {
        menu->handleInput(window);
    }

    if (!menu->isEditorModeActive() && !menu->isCommandChatActive()) {
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
    glClearColor(0.03f, 0.10f, 0.18f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    int width, height;
    glfwGetWindowSize(getWindow(), &width, &height);
    glViewport(0, 0, width, height);

    projection = glm::perspective(glm::radians(camera->Zoom), (float)width / (float)height, 0.1f, 200.0f);
    view = camera->getViewMatrix();
    const float sceneTime = static_cast<float>(glfwGetTime());

    // === ПРОВЕРКА ИЗМЕНЕНИЯ ТРАНСФОРМАЦИИ И ПЕРЕСЧЕТ LBVH ===
    static glm::mat4 lastModelMatrix = glm::mat4(0.0f);
    glm::mat4 currentModelMatrix = menu->getModelMatrix();

    if (lastModelMatrix != currentModelMatrix || menu->rebuildLBVH) {
        if (model && !model->meshes.empty()) {
            std::vector<glm::vec3> positions;
            std::vector<uint32_t> indices;
            const auto& meshLocalMatrices = model->getMeshLocalMatrices();

            for (size_t i = 0; i < model->meshes.size(); ++i) {
                const auto& mesh = model->meshes[i];
                const glm::mat4 meshLocal = (i < meshLocalMatrices.size()) ? meshLocalMatrices[i] : glm::mat4(1.0f);
                const uint32_t vertexOffset = static_cast<uint32_t>(positions.size());
                for (const auto& vertex : mesh.vertices) {
                    positions.push_back(glm::vec3(meshLocal * glm::vec4(vertex.position, 1.0f))); // Координаты в пространстве модели
                }
                for (const auto& index : mesh.indices) {
                    indices.push_back(vertexOffset + index);
                }
            }

            // Применяем текущую трансформацию
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

    renderEnvironment(width, height, sceneTime);
    menu->render(view, projection);

    glm::mat4 modelMatrix = menu->getModelMatrix();
    glm::vec3 modelPosition = menu->getModelPosition();
    glm::vec3 modelRotation = menu->getModelRotation();
    glm::vec3 modelScale = menu->getModelScale();

    bool wireframe = menu->isWireframeMode();
    bool showNormals = menu->isShowNormals();
    bool geometryEffects = menu->isGeometryEffects();
    bool showLBVH = menu->showLBVH;

    if (!menu->isEditorModeActive() && !menu->isCommandChatActive()) {
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
        glDepthMask(GL_TRUE);
    }
    else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
    }

    if (model && !model->meshes.empty()) {
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        try {
            shader->use();
            shader->setMat4("view", view);
            shader->setMat4("projection", projection);
            shader->setVec3("camPos", camera->Position);
            shader->setFloat("bunnySoftness", menu->getBunnySoftness());
            shader->setFloat("geometryEffectStrength", geometryEffects ? 1.0f : 0.0f);
            GLint timeLocation = glGetUniformLocation(shader->ID, "time");
            if (timeLocation != -1) {
                shader->setFloat("time", sceneTime);
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
                    normalsShader->setFloat("time", sceneTime);
                }
                model->Draw(*normalsShader, *camera, modelMatrix);
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
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    if (font) {
        try {
            textRender->use();
            glm::mat4 textProjection = glm::ortho(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, -1.0f, 1.0f);
            textRender->setMat4("projection", textProjection);
            textRender->setInt("image", 0);

            // Системная информация
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

            // Информация о модели
            if (model && !model->meshes.empty()) {
                textRender->setVec3("Color", glm::vec3(0.0f, 0.8f, 1.0f));
                std::string meshInfo = "Cloud Bunny | Meshes: " + std::to_string(model->meshes.size()) +
                    " | Vertices: " + std::to_string(model->meshes[0].vertices.size());
                font->print(meshInfo.c_str(), 10.0f, 130.0f, 1.0f, glm::vec3(0.0f, 0.8f, 1.0f));

                textRender->setVec3("Color", glm::vec3(0.8f, 1.0f, 0.8f));
                std::string glassInfo = "SOFT CLOUD BUNNY ACTIVE";
                font->print(glassInfo.c_str(), 10.0f, 105.0f, 1.0f, glm::vec3(0.8f, 1.0f, 0.8f));

                std::string cloudInfo = "Clouds Dens:" + std::to_string(menu->getCloudDensity()).substr(0, 4) +
                    " Soft:" + std::to_string(menu->getCloudSoftness()).substr(0, 4) +
                    " Storm:" + std::to_string(menu->getCloudStorminess()).substr(0, 4) +
                    " Cov:" + std::to_string(menu->getCloudCoverage()).substr(0, 4) +
                    " Wave:" + std::to_string(menu->getOceanWaveStrength()).substr(0, 4) +
                    " UW:" + std::to_string(menu->getUnderwaterDensity()).substr(0, 4);
                font->print(cloudInfo.c_str(), 10.0f, 210.0f, 0.8f, glm::vec3(0.7f, 0.9f, 1.0f));

                std::string skyInfo = "Sky T:" + std::to_string(menu->getSkyTimeHours()).substr(0, 5) +
                    "h Moon:" + std::to_string(menu->getMoonBrightness()).substr(0, 4) +
                    " Stars:" + std::to_string(menu->getStarBrightness()).substr(0, 4);
                font->print(skyInfo.c_str(), 10.0f, 260.0f, 0.8f, glm::vec3(0.82f, 0.86f, 1.0f));

                std::string rotInfo = "Rotations X:" + std::to_string((int)modelRotation.x) +
                    " Y:" + std::to_string((int)modelRotation.y) +
                    " Z:" + std::to_string((int)modelRotation.z) +
                    " Scale:" + std::to_string(modelScale.x);
                font->print(rotInfo.c_str(), 10.0f, 155.0f, 0.8f, glm::vec3(1.0f, 1.0f, 0.0f));

                // ВАЖНАЯ ОТЛАДОЧНАЯ ИНФОРМАЦИЯ ДЛЯ LBVH
                if (bvh->numInternalNodes > 0) {
                    textRender->setVec3("Color", glm::vec3(1.0f, 0.0f, 1.0f));
                    std::string lbvhDebug = "LBVH Nodes: " + std::to_string(bvh->numInternalNodes) +
                        " VBO: " + std::to_string(bvh->aabbInstanceVBO);
                    font->print(lbvhDebug.c_str(), 10.0f, 235.0f, 0.8f, glm::vec3(1.0f, 0.0f, 1.0f));
                }
            }

            // Статусы режимов
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

            // Информация о редакторе
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

            // Инструкции
            textRender->setVec3("Color", glm::vec3(0.7f, 0.7f, 0.7f));
            std::string controlsText = "Controls: WASD+Mouse | Space/Shift-Up/Down | T-Wireframe | N-Normals | G-GeomFX | L-LBVH | B-Editor | TAB-Chat(day/night) | ESC-Exit";
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
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
}
