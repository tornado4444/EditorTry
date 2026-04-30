#include "Menu.hpp"
#include <iostream>
#include <algorithm>
#include <cctype>
#include <cstring>

Menu::Menu() :
    showMainMenuBar(true),
    showDemo(false),
    showAbout(false),
    showFileDialog(false),
    showViewSettings(false),
    showRenderSettings(false),
    editorMode(false),
    bKeyPressed(false),
    wireframeMode(false),
    showNormals(false),
    geometryEffects(false),
    showLBVH(false),
    rebuildLBVH(false),
    lastLBVHBuildTime(0.0f),
    windowPtr(nullptr),
    modelPosition(0.0f, 1.15f, -8.0f),
    modelRotation(90.0f, 180.0f, 0.0f),
    modelScale(10.0f),
    guizmoOperation(ImGuizmo::TRANSLATE),
    guizmoMode(ImGuizmo::WORLD),
    useSnap(false),
    modelSelected(false),
    cloudDensity(1.0f),
    cloudSoftness(0.72f),
    cloudStorminess(0.18f),
    bunnySoftness(0.82f),
    oceanWaveStrength(1.45f),
    underwaterDensity(1.25f),
    skyTimeHours(21.0f),
    dayNightSpeed(0.75f),
    animateDayNight(false),
    starBrightness(1.0f),
    moonBrightness(1.35f),
    moonSizeDegrees(0.60f),
    cloudCoverage(1.0f),
    commandChatOpen(false),
    commandChatFocusRequested(false),
    tabPressed(false),
    commandFeedbackUntil(0.0),
    localMinBounds(glm::vec3(0.0f)),
    localMaxBounds(glm::vec3(0.0f)) {
    snapValues[0] = 1.0f;
    snapValues[1] = 15.0f;
    snapValues[2] = 0.1f;
    commandBuffer[0] = '\0';
    updateModelMatrix();
}

void Menu::setModelBounds(const glm::vec3& min, const glm::vec3& max) {
    localMinBounds = min;
    localMaxBounds = max;
    glm::vec3 size = max - min;
    glm::vec3 center = (max + min) * 0.5f;
}

void Menu::setModelSelected(bool selected) {
    modelSelected = selected;
}

Menu::~Menu() {
    shutdown();
}

bool Menu::initialize(GLFWwindow* window) {
    windowPtr = window;
    ImGuizmo::SetGizmoSizeClipSpace(10.0f);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 5.0f;
    style.Colors[ImGuiCol_WindowBg].w = 0.95f;

    if (!ImGui_ImplGlfw_InitForOpenGL(window, false)) {
        std::cerr << "Failed to initialize ImGui GLFW backend!" << std::endl;
        return false;
    }

    if (!ImGui_ImplOpenGL3_Init("#version 460 core")) {
        std::cerr << "Failed to initialize ImGui OpenGL backend!" << std::endl;
        return false;
    }

    std::cout << "ImGui menu system with ImGuizmo initialized successfully!" << std::endl;
    return true;
}

void Menu::updateModelMatrix() {
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, modelPosition);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(modelRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(modelRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(modelRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    modelMatrix = glm::scale(modelMatrix, modelScale);
}

void Menu::toggleEditorMode() {
    editorMode = !editorMode;
    if (editorMode) {
        glfwSetInputMode(windowPtr, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    else {
        glfwSetInputMode(windowPtr, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
}

void Menu::handleEditorToggle(GLFWwindow* window) {
    bool bKeyCurrentlyPressed = (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS);
    if (bKeyCurrentlyPressed && !bKeyPressed) {
        toggleEditorMode();
    }
    bKeyPressed = bKeyCurrentlyPressed;
}

void Menu::handleInput(GLFWwindow* window) {
    static bool mKeyPressed = false;
    bool mKeyCurrentlyPressed = (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS);
    if (mKeyCurrentlyPressed && !mKeyPressed) {
        modelSelected = true;
        std::cout << "*** MODEL FORCE SELECTED WITH M KEY! ***" << std::endl;
    }
    mKeyPressed = mKeyCurrentlyPressed;

    static bool lKeyPressed = false;
    bool lKeyCurrentlyPressed = (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS);
    if (lKeyCurrentlyPressed && !lKeyPressed) {
        showLBVH = !showLBVH;
        std::cout << "LBVH visualization: " << (showLBVH ? "ON" : "OFF") << std::endl;
    }
    lKeyPressed = lKeyCurrentlyPressed;

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        guizmoOperation = ImGuizmo::TRANSLATE;
        std::cout << "Set operation to TRANSLATE" << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
        guizmoOperation = ImGuizmo::ROTATE;
        std::cout << "Set operation to ROTATE" << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
        guizmoOperation = ImGuizmo::SCALE;
        std::cout << "Set operation to SCALE" << std::endl;
    }
}

bool Menu::wantsMouseInput() const {
    ImGuiIO& io = ImGui::GetIO();
    if (commandChatOpen) {
        return io.WantCaptureMouse;
    }
    if (!editorMode) return false;
    bool imguiWantsMouse = io.WantCaptureMouse;
    bool guizmoIsOver = ImGuizmo::IsOver();
    bool guizmoIsUsing = ImGuizmo::IsUsing();
    return imguiWantsMouse || guizmoIsOver || guizmoIsUsing;
}

bool Menu::wantsKeyboardInput() const {
    ImGuiIO& io = ImGui::GetIO();
    if (commandChatOpen) {
        return io.WantCaptureKeyboard;
    }
    if (!editorMode) return false;
    return io.WantCaptureKeyboard;
}

void Menu::renderGuizmo(const glm::mat4& view, const glm::mat4& projection) {
    if (!editorMode || !modelSelected) return;
    ImGuiIO& io = ImGui::GetIO();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
    ImGuizmo::Enable(true);
    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetGizmoSizeClipSpace(0.1f);

    glm::mat4 currentMatrix = modelMatrix;
    bool manipulated = ImGuizmo::Manipulate(
        glm::value_ptr(view),
        glm::value_ptr(projection),
        guizmoOperation,
        guizmoMode,
        glm::value_ptr(currentMatrix),
        nullptr,
        useSnap ? snapValues : nullptr,
        nullptr,
        nullptr
    );

    if (manipulated) {
        glm::vec3 translation, rotation, scale;
        ImGuizmo::DecomposeMatrixToComponents(
            glm::value_ptr(currentMatrix),
            glm::value_ptr(translation),
            glm::value_ptr(rotation),
            glm::value_ptr(scale)
        );
        modelPosition = translation;
        modelRotation = rotation;
        modelScale = scale;
        modelMatrix = currentMatrix;
        rebuildLBVH = true; 
    }
}

void Menu::debugGuizmoState() {
    if (!editorMode) return;
    static int counter = 0;
    if (counter++ % 60 == 0) {
        ImGuiIO& io = ImGui::GetIO();
        std::cout << "Display Size: " << io.DisplaySize.x << "x" << io.DisplaySize.y << std::endl;
        std::cout << "Mouse Pos: " << io.MousePos.x << ", " << io.MousePos.y << std::endl;
        std::cout << "WantCaptureMouse: " << io.WantCaptureMouse << std::endl;
    }
}

void Menu::render(const glm::mat4& view, const glm::mat4& projection) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();

    if (animateDayNight) {
        skyTimeHours += dayNightSpeed * ImGui::GetIO().DeltaTime;
        while (skyTimeHours >= 24.0f) {
            skyTimeHours -= 24.0f;
        }
        while (skyTimeHours < 0.0f) {
            skyTimeHours += 24.0f;
        }
    }

    if (editorMode) {
        if (showMainMenuBar) renderMainMenuBar();
        if (showAbout) renderAboutWindow();
        if (showViewSettings) renderViewSettings();
        if (showRenderSettings) renderRenderSettings();
        if (showDemo) ImGui::ShowDemoWindow(&showDemo);
        renderEditorPanel();
        renderGuizmoControls();
    }

    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 250, 10), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(240, 100), ImGuiCond_Always);

    ImGui::Begin("Mode Indicator", nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBackground);

    if (editorMode) {
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "EDITOR MODE ON");
        ImGui::Text("Press B to exit");
        if (modelSelected) {
            if (ImGuizmo::IsUsing()) {
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Guizmo: TRANSFORMING");
            }
            else if (ImGuizmo::IsOver()) {
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Guizmo: HOVER");
            }
            else {
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Guizmo: READY");
            }
        }
        else {
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Guizmo: CLICK MODEL");
        }
        if (showLBVH) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "LBVH: ON (%.2f ms)", lastLBVHBuildTime);
        }
        else {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "LBVH: OFF");
        }
    }
    else {
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.2f, 1.0f), "GAME MODE");
        ImGui::Text("Press B for editor");
    }

    ImGui::End();
    renderCommandChat();
    if (editorMode) renderGuizmo(view, projection);
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Menu::renderGuizmoControls() {
    ImGui::SetNextWindowPos(ImVec2(10, 450), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 250), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Guizmo Controls")) {
        ImGui::Text("Transform Operations:");
        if (ImGui::RadioButton("Translate (1)", guizmoOperation == ImGuizmo::TRANSLATE)) {
            guizmoOperation = ImGuizmo::TRANSLATE;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Rotate (2)", guizmoOperation == ImGuizmo::ROTATE)) {
            guizmoOperation = ImGuizmo::ROTATE;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Scale (3)", guizmoOperation == ImGuizmo::SCALE)) {
            guizmoOperation = ImGuizmo::SCALE;
        }
        ImGui::Separator();
        ImGui::Text("Mode:");
        if (ImGui::RadioButton("World", guizmoMode == ImGuizmo::WORLD)) {
            guizmoMode = ImGuizmo::WORLD;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Local", guizmoMode == ImGuizmo::LOCAL)) {
            guizmoMode = ImGuizmo::LOCAL;
        }
        ImGui::Separator();
        ImGui::Checkbox("Use Snap", &useSnap);
        if (useSnap) {
            switch (guizmoOperation) {
            case ImGuizmo::TRANSLATE:
                ImGui::DragFloat("Snap Translate", &snapValues[0], 0.1f);
                break;
            case ImGuizmo::ROTATE:
                ImGui::DragFloat("Snap Rotate", &snapValues[1], 1.0f);
                break;
            case ImGuizmo::SCALE:
                ImGui::DragFloat("Snap Scale", &snapValues[2], 0.01f);
                break;
            }
        }
        ImGui::Separator();
        ImGui::Text("Current Transform:");
        ImGui::Text("Pos: %.2f, %.2f, %.2f", modelPosition.x, modelPosition.y, modelPosition.z);
        ImGui::Text("Rot: %.2f, %.2f, %.2f", modelRotation.x, modelRotation.y, modelRotation.z);
        ImGui::Text("Scale: %.2f, %.2f, %.2f", modelScale.x, modelScale.y, modelScale.z);
        if (ImGui::Button("Reset Transform")) {
            modelPosition = glm::vec3(0.0f, 1.15f, -8.0f);
            modelRotation = glm::vec3(90.0f, 180.0f, 0.0f);
            modelScale = glm::vec3(10.0f);
            updateModelMatrix();
            rebuildLBVH = true;
        }
        ImGui::Separator();
        ImGui::Text("LBVH Controls:");
        ImGui::Checkbox("Show LBVH (L)", &showLBVH);
        if (ImGui::Button("Rebuild LBVH")) {
            rebuildLBVH = true;
        }
        ImGui::Text("Last LBVH Build Time: %.2f ms", lastLBVHBuildTime);
    }
    ImGui::End();
}

void Menu::renderMainMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            renderFileMenu();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            renderViewMenu();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Tools")) {
            if (ImGui::MenuItem("Render Settings", "Ctrl+R")) {
                showRenderSettings = !showRenderSettings;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("ImGui Demo", "F12")) {
                showDemo = !showDemo;
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {
                showAbout = true;
            }
            ImGui::EndMenu();
        }
        ImGui::SameLine(ImGui::GetWindowWidth() - 120);
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::EndMainMenuBar();
    }
}

void Menu::renderFileMenu() {
    if (ImGui::MenuItem("New Scene", "Ctrl+N")) {
        std::cout << "New Scene selected" << std::endl;
    }
    if (ImGui::MenuItem("Open Scene", "Ctrl+O")) {
        std::cout << "Open Scene selected" << std::endl;
    }
    if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {
        std::cout << "Save Scene selected" << std::endl;
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Exit Editor Mode", "B")) {
        toggleEditorMode();
    }
    if (ImGui::MenuItem("Exit Application", "Alt+F4")) {
        glfwSetWindowShouldClose(windowPtr, true);
    }
}

void Menu::renderViewMenu() {
    if (ImGui::MenuItem("View Settings", "V")) {
        showViewSettings = !showViewSettings;
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Wireframe Mode", "T", wireframeMode)) {
        wireframeMode = !wireframeMode;
    }
    if (ImGui::MenuItem("Show Normals", "N", showNormals)) {
        showNormals = !showNormals;
    }
    if (ImGui::MenuItem("Geometry Effects", "G", geometryEffects)) {
        geometryEffects = !geometryEffects;
    }
    if (ImGui::MenuItem("Show LBVH", "L", showLBVH)) {
        showLBVH = !showLBVH;
    }
}

void Menu::renderAboutWindow() {
    if (!ImGui::Begin("About Glass Bunny Engine", &showAbout)) {
        ImGui::End();
        return;
    }
    ImGui::Text("Glass Bunny Engine v1.0 with ImGuizmo and LBVH");
    ImGui::Separator();
    ImGui::Text("OpenGL 3D Rendering Engine");
    ImGui::Text("Features:");
    ImGui::BulletText("GLTF Model Loading");
    ImGui::BulletText("ImGuizmo Transform Gizmos");
    ImGui::BulletText("Real-time Ocean + Volumetric Clouds");
    ImGui::BulletText("Soft Cloud-like Bunny Material");
    ImGui::BulletText("Linear BVH Construction (GPU)");
    ImGui::BulletText("Editor Mode (Press B)");
    ImGui::Separator();
    ImGui::Text("Guizmo Controls:");
    ImGui::BulletText("1 - Translate Mode");
    ImGui::BulletText("2 - Rotate Mode");
    ImGui::BulletText("3 - Scale Mode");
    ImGui::BulletText("Drag gizmo to transform object");
    ImGui::Separator();
    ImGui::Text("LBVH Controls:");
    ImGui::BulletText("L - Toggle LBVH visualization");
    ImGui::BulletText("Rebuild in Guizmo Controls panel");
    if (ImGui::Button("Close")) {
        showAbout = false;
    }
    ImGui::End();
}

void Menu::renderViewSettings() {
    if (!ImGui::Begin("View Settings", &showViewSettings)) {
        ImGui::End();
        return;
    }
    ImGui::Text("Rendering Options");
    ImGui::Separator();
    ImGui::Checkbox("Wireframe Mode", &wireframeMode);
    ImGui::Checkbox("Show Normals", &showNormals);
    ImGui::Checkbox("Geometry Effects", &geometryEffects);
    ImGui::Checkbox("Show LBVH", &showLBVH);
    ImGui::End();
}

void Menu::forceSelectModel() {
    modelSelected = true;
    std::cout << "Model force selected for testing!" << std::endl;
}

void Menu::renderRenderSettings() {
    if (!ImGui::Begin("Render Settings", &showRenderSettings)) {
        ImGui::End();
        return;
    }
    ImGui::Text("Advanced Rendering Settings");
    ImGui::Separator();
    static float clearColor[4] = { 0.07f, 0.13f, 0.17f, 1.0f };
    ImGui::ColorEdit4("Clear Color", clearColor);
    static bool vsync = true;
    if (ImGui::Checkbox("V-Sync", &vsync)) {
        glfwSwapInterval(vsync ? 1 : 0);
    }
    ImGui::Separator();
    ImGui::Text("Atmosphere (single mode)");
    ImGui::SliderFloat("Cloud Density", &cloudDensity, 0.2f, 2.8f, "%.2f");
    ImGui::SliderFloat("Cloud Softness", &cloudSoftness, 0.0f, 1.0f, "%.2f");
    ImGui::SliderFloat("Cloud Storminess", &cloudStorminess, 0.0f, 1.0f, "%.2f");
    ImGui::SliderFloat("Cloud Coverage", &cloudCoverage, 0.0f, 2.0f, "%.2f");
    ImGui::SliderFloat("Ocean Wave Strength", &oceanWaveStrength, 0.4f, 2.8f, "%.2f");
    ImGui::SliderFloat("Underwater Density", &underwaterDensity, 0.3f, 2.5f, "%.2f");
    ImGui::SliderFloat("Bunny Softness", &bunnySoftness, 0.0f, 1.0f, "%.2f");
    ImGui::Separator();
    ImGui::Text("NASA Night Sky");
    ImGui::SliderFloat("Sky Time (hours)", &skyTimeHours, 0.0f, 24.0f, "%.2f");
    if (ImGui::Button("Set DAY")) {
        skyTimeHours = 13.0f;
        animateDayNight = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Set NIGHT")) {
        skyTimeHours = 23.0f;
        animateDayNight = false;
    }
    ImGui::Checkbox("Auto Day/Night Cycle", &animateDayNight);
    ImGui::SliderFloat("Day/Night Speed", &dayNightSpeed, 0.01f, 8.0f, "%.2f");
    ImGui::SliderFloat("Star Brightness", &starBrightness, 0.0f, 4.0f, "%.2f");
    ImGui::SliderFloat("Moon Brightness", &moonBrightness, 0.0f, 4.0f, "%.2f");
    ImGui::SliderFloat("Moon Size (deg)", &moonSizeDegrees, 0.12f, 3.5f, "%.2f");
    ImGui::Text("Frame Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::End();
}

void Menu::handleGlobalInput(GLFWwindow* window) {
    bool tabCurrentlyPressed = (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS);
    if (tabCurrentlyPressed && !tabPressed) {
        commandChatOpen = !commandChatOpen;
        commandChatFocusRequested = commandChatOpen;
        if (commandChatOpen) {
            commandBuffer[0] = '\0';
        }
    }
    tabPressed = tabCurrentlyPressed;
}

void Menu::renderEditorPanel() {
    ImGui::SetNextWindowPos(ImVec2(10, 30), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(320, 560), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Editor Tools")) {
        ImGui::Text("Glass Bunny Editor with ImGuizmo and LBVH");
        ImGui::Separator();
        ImGui::Text("Quick Controls:");
        if (ImGui::Button("Toggle Wireframe (T)")) {
            wireframeMode = !wireframeMode;
        }
        ImGui::SameLine();
        if (ImGui::Button("Show Normals (N)")) {
            showNormals = !showNormals;
        }
        if (ImGui::Button("Geometry Effects (G)")) {
            geometryEffects = !geometryEffects;
        }
        if (ImGui::Button("Show LBVH (L)")) {
            showLBVH = !showLBVH;
        }
        ImGui::Separator();
        ImGui::Text("Ocean + Volumetric Clouds (single category)");
        ImGui::SliderFloat("Cloud Density", &cloudDensity, 0.2f, 2.8f, "%.2f");
        ImGui::SliderFloat("Cloud Softness", &cloudSoftness, 0.0f, 1.0f, "%.2f");
        ImGui::SliderFloat("Cloud Storminess", &cloudStorminess, 0.0f, 1.0f, "%.2f");
        ImGui::SliderFloat("Cloud Coverage", &cloudCoverage, 0.0f, 2.0f, "%.2f");
        ImGui::SliderFloat("Ocean Wave Strength", &oceanWaveStrength, 0.4f, 2.8f, "%.2f");
        ImGui::SliderFloat("Underwater Density", &underwaterDensity, 0.3f, 2.5f, "%.2f");
        ImGui::SliderFloat("Bunny Softness", &bunnySoftness, 0.0f, 1.0f, "%.2f");
        ImGui::Separator();
        ImGui::Text("Moon / Night / Stars");
        ImGui::SliderFloat("Sky Time (hours)", &skyTimeHours, 0.0f, 24.0f, "%.2f");
        if (ImGui::Button("Set DAY")) {
            skyTimeHours = 13.0f;
            animateDayNight = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Set NIGHT")) {
            skyTimeHours = 23.0f;
            animateDayNight = false;
        }
        ImGui::Checkbox("Auto Day/Night Cycle", &animateDayNight);
        ImGui::SliderFloat("Day/Night Speed", &dayNightSpeed, 0.01f, 8.0f, "%.2f");
        ImGui::SliderFloat("Star Brightness", &starBrightness, 0.0f, 4.0f, "%.2f");
        ImGui::SliderFloat("Moon Brightness", &moonBrightness, 0.0f, 4.0f, "%.2f");
        ImGui::SliderFloat("Moon Size (deg)", &moonSizeDegrees, 0.12f, 3.5f, "%.2f");
        ImGui::Text("Command Chat: TAB -> type 'day' or 'night'");
        ImGui::Separator();
        ImGui::Text("Transform with ImGuizmo:");
        ImGui::Text("Use 1/2/3 keys to switch modes");
        ImGui::Text("Drag the gizmo in viewport");
        ImGui::Separator();
        ImGui::Text("LBVH Controls:");
        if (ImGui::Button("Rebuild LBVH")) {
            rebuildLBVH = true;
        }
        ImGui::Text("Last LBVH Build: %.2f ms", lastLBVHBuildTime);
        ImGui::Separator();
        if (ImGui::Button("Exit Editor (B)")) {
            toggleEditorMode();
        }
    }
    ImGui::End();
}

void Menu::executeCommand(const std::string& rawCommand) {
    std::string command = rawCommand;

    auto notSpace = [](unsigned char ch) { return !std::isspace(ch); };
    command.erase(command.begin(), std::find_if(command.begin(), command.end(), notSpace));
    command.erase(std::find_if(command.rbegin(), command.rend(), notSpace).base(), command.end());
    std::transform(command.begin(), command.end(), command.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    if (command == "day") {
        skyTimeHours = 13.0f;
        animateDayNight = false;
        commandFeedback = "Time set to DAY";
    }
    else if (command == "night") {
        skyTimeHours = 23.0f;
        animateDayNight = false;
        commandFeedback = "Time set to NIGHT";
    }
    else if (command == "cycle") {
        animateDayNight = !animateDayNight;
        commandFeedback = animateDayNight ? "Auto cycle ON" : "Auto cycle OFF";
    }
    else if (command.empty()) {
        commandFeedback = "Empty command";
    }
    else {
        commandFeedback = "Unknown command. Use: day | night | cycle";
    }

    commandFeedbackUntil = ImGui::GetTime() + 3.5;
}

void Menu::renderCommandChat() {
    const double now = ImGui::GetTime();
    ImGuiIO& io = ImGui::GetIO();

    if (commandChatOpen) {
        ImGui::SetNextWindowPos(ImVec2(14.0f, io.DisplaySize.y - 110.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(440.0f, 94.0f), ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.68f);

        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse;

        ImGui::Begin("Command Chat", nullptr, flags);
        ImGui::TextColored(ImVec4(0.95f, 0.95f, 0.95f, 1.0f), "Commands: day | night | cycle");
        ImGui::PushItemWidth(-1.0f);

        if (commandChatFocusRequested) {
            ImGui::SetKeyboardFocusHere();
            commandChatFocusRequested = false;
        }

        if (ImGui::InputText("##command_chat", commandBuffer, IM_ARRAYSIZE(commandBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
            executeCommand(commandBuffer);
            commandBuffer[0] = '\0';
            commandChatOpen = false;
        }
        ImGui::PopItemWidth();

        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            commandChatOpen = false;
            commandChatFocusRequested = false;
        }

        ImGui::End();
    }

    if (!commandFeedback.empty() && now < commandFeedbackUntil) {
        ImGui::SetNextWindowPos(ImVec2(14.0f, io.DisplaySize.y - 145.0f), ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.42f);
        ImGuiWindowFlags feedbackFlags =
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_AlwaysAutoResize;
        ImGui::Begin("Command Feedback", nullptr, feedbackFlags);
        ImGui::TextColored(ImVec4(0.92f, 0.95f, 1.0f, 1.0f), "%s", commandFeedback.c_str());
        ImGui::End();
    }
}

void Menu::shutdown() {
    if (ImGui::GetCurrentContext()) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        std::cout << "ImGui menu system shut down" << std::endl;
    }
}
