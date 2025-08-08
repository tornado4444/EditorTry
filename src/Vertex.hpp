#pragma once
#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;        
    glm::vec2 texCoords;    

    glm::vec3 tangent = glm::vec3(0.0f);
    glm::vec3 bitangent = glm::vec3(0.0f);

    Vertex() = default;

    Vertex(glm::vec3 pos, glm::vec3 norm = glm::vec3(0.0f), glm::vec3 col = glm::vec3(1.0f), glm::vec2 tex = glm::vec2(0.0f))
        : position(pos), normal(norm), color(col), texCoords(tex) {
    }
};