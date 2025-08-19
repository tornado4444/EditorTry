#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <ImGuizmo.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include "LBVH.hpp"

class Menu {
public:
    Menu();
    ~Menu();

    bool initialize(GLFWwindow* window);
    void render(const glm::mat4& view, const glm::mat4& projection);
    void shutdown();

    void toggleEditorMode();
    void handleEditorToggle(GLFWwindow* window);
    void handleInput(GLFWwindow* window);
    void forceSelectModel();

    bool wantsMouseInput() const;
    bool wantsKeyboardInput() const;
    bool isEditorModeActive() const { return editorMode; }

    void renderGuizmo(const glm::mat4& view, const glm::mat4& projection);
    bool isGuizmoActive() const { return ImGuizmo::IsOver() || ImGuizmo::IsUsing(); }
    bool isGuizmoUsing() const { return ImGuizmo::IsUsing(); }
    bool isGuizmoOver() const { return ImGuizmo::IsOver(); }
    void debugGuizmoState();

    glm::mat4 getModelMatrix() const { return modelMatrix; }
    glm::vec3 getModelPosition() const { return modelPosition; }
    glm::vec3 getModelRotation() const { return modelRotation; }
    glm::vec3 getModelScale() const { return modelScale; }
    glm::vec3 localMinBounds;
    glm::vec3 localMaxBounds;
    bool modelSelected;

    bool isWireframeMode() const { return wireframeMode; }
    bool isShowNormals() const { return showNormals; }
    bool isGeometryEffects() const { return geometryEffects; }

    void setWireframeMode(bool mode) { wireframeMode = mode; }
    void setShowNormals(bool show) { showNormals = show; }
    void setGeometryEffects(bool effects) { geometryEffects = effects; }
    void setModelBounds(const glm::vec3& min, const glm::vec3& max);
    void setModelSelected(bool selected);

    void updateModelMatrix();
public:
    bool showMainMenuBar;
    bool showDemo;
    bool showAbout;
    bool showFileDialog;
    bool showViewSettings;
    bool showRenderSettings;
    bool editorMode;
    bool bKeyPressed;
    bool showLBVH;
    bool rebuildLBVH;
    bool wireframeMode;
    bool showNormals;
    bool geometryEffects;

    GLFWwindow* windowPtr;

    glm::vec3 modelPosition;
    glm::vec3 modelRotation;
    glm::vec3 modelScale;
    glm::mat4 modelMatrix;
    float lastLBVHBuildTime;

    ImGuizmo::OPERATION guizmoOperation;
    ImGuizmo::MODE guizmoMode;
    bool useSnap;
    float snapValues[3];

    void renderMainMenuBar();
    void renderFileMenu();
    void renderViewMenu();
    void renderAboutWindow();
    void renderViewSettings();
    void renderRenderSettings();
    void renderEditorPanel();
    void renderGuizmoControls();
};