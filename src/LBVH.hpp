#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <Shader.hpp>

struct AABB {
    glm::vec3 min = glm::vec3(FLT_MAX);
    glm::vec3 max = glm::vec3(-FLT_MAX);
    void expand(const glm::vec3& point) {
        min = glm::min(min, point);
        max = glm::max(max, point);
    }
};

struct Primitive {
    AABB aabb;
    uint32_t index;
};

struct LBVHNode {
    int left, right;
    uint32_t primitiveIdx;
    float aabbMinX, aabbMinY, aabbMinZ;
    float aabbMaxX, aabbMaxY, aabbMaxZ;
};

struct MortonCodeElement {
    uint32_t mortonCode;
    uint32_t elementIdx;
};

struct LBVHConstructionInfo {
    uint32_t parent;
    int visitationCount;
};

class BVH {
public:
    std::vector<Primitive> primitives;
    std::vector<LBVHNode> m_bvh;
    std::vector<MortonCodeElement> mortonCodes;
    GLuint aabbInstanceVBO = 0;
    uint32_t numInternalNodes = 0;

    BVH() = default;
    ~BVH() {
        if (aabbInstanceVBO) {
            glDeleteBuffers(1, &aabbInstanceVBO);
            aabbInstanceVBO = 0;
        }
    }

    void buildLBVHDynamic(const std::vector<glm::vec3>& positions, const std::vector<uint32_t>& indices,
        Shader* mortonShader, Shader* sortShader, Shader* hierarchyShader, Shader* aabbShader);
};