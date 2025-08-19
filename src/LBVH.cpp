#include "LBVH.hpp"
#include <algorithm>
#include <vector>

void BVH::buildLBVHDynamic(const std::vector<glm::vec3>& positions, const std::vector<uint32_t>& indices,
    Shader* mortonShader, Shader* sortShader, Shader* hierarchyShader, Shader* aabbShader) {

    m_bvh.clear();
    primitives.clear();
    mortonCodes.clear();

    uint32_t numTris = indices.size() / 3;
    if (numTris == 0) {
        MyglobalLogger().logMessage(Logger::ERROR, "No triangles to build LBVH", __FILE__, __LINE__);
        return;
    }

    MyglobalLogger().logMessage(Logger::INFO, "Building LBVH for " + std::to_string(numTris) + " triangles", __FILE__, __LINE__);

    // Проверяем контекст OpenGL
    if (!glfwGetCurrentContext()) {
        MyglobalLogger().logMessage(Logger::ERROR, "No OpenGL context available!", __FILE__, __LINE__);
        return;
    }

    AABB globalAABB;
    primitives.resize(numTris);

    struct Element {
        uint32_t primitiveIdx;
        float aabbMinX, aabbMinY, aabbMinZ;
        float aabbMaxX, aabbMaxY, aabbMaxZ;
    };
    std::vector<Element> gpuElements(numTris);

    for (size_t i = 0; i < numTris; ++i) {
        uint32_t idx0 = indices[i * 3];
        uint32_t idx1 = indices[i * 3 + 1];
        uint32_t idx2 = indices[i * 3 + 2];

        if (idx0 >= positions.size() || idx1 >= positions.size() || idx2 >= positions.size()) {
            MyglobalLogger().logMessage(Logger::ERROR,
                "Invalid triangle indices at " + std::to_string(i) +
                ": [" + std::to_string(idx0) + "," + std::to_string(idx1) + "," + std::to_string(idx2) +
                "] with positions.size()=" + std::to_string(positions.size()), __FILE__, __LINE__);
            return;
        }

        glm::vec3 v0 = positions[idx0];
        glm::vec3 v1 = positions[idx1];
        glm::vec3 v2 = positions[idx2];

        primitives[i].aabb.min = glm::vec3(FLT_MAX);
        primitives[i].aabb.max = glm::vec3(-FLT_MAX);
        primitives[i].aabb.expand(v0);
        primitives[i].aabb.expand(v1);
        primitives[i].aabb.expand(v2);
        primitives[i].index = i;

        gpuElements[i].primitiveIdx = i;
        gpuElements[i].aabbMinX = primitives[i].aabb.min.x;
        gpuElements[i].aabbMinY = primitives[i].aabb.min.y;
        gpuElements[i].aabbMinZ = primitives[i].aabb.min.z;
        gpuElements[i].aabbMaxX = primitives[i].aabb.max.x;
        gpuElements[i].aabbMaxY = primitives[i].aabb.max.y;
        gpuElements[i].aabbMaxZ = primitives[i].aabb.max.z;

        globalAABB.expand(v0);
        globalAABB.expand(v1);
        globalAABB.expand(v2);
    }

    MyglobalLogger().logMessage(Logger::DEBUG,
        "Global AABB: min=(" + std::to_string(globalAABB.min.x) + "," +
        std::to_string(globalAABB.min.y) + "," + std::to_string(globalAABB.min.z) +
        "), max=(" + std::to_string(globalAABB.max.x) + "," +
        std::to_string(globalAABB.max.y) + "," + std::to_string(globalAABB.max.z) + ")", __FILE__, __LINE__);

    glm::vec3 extent = globalAABB.max - globalAABB.min;
    if (glm::all(glm::lessThanEqual(extent, glm::vec3(0.0001f)))) {
        MyglobalLogger().logMessage(Logger::ERROR,
            "Degenerate global AABB - all vertices are at the same location! Extent: (" +
            std::to_string(extent.x) + "," + std::to_string(extent.y) + "," + std::to_string(extent.z) + ")",
            __FILE__, __LINE__);
        return;
    }

    GLuint elemBuffer, mortonBuffer;
    glGenBuffers(1, &elemBuffer);
    glGenBuffers(1, &mortonBuffer);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, elemBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, numTris * sizeof(Element), gpuElements.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, mortonBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, numTris * sizeof(MortonCodeElement), nullptr, GL_DYNAMIC_COPY);

    glUseProgram(mortonShader->ID);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, elemBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mortonBuffer);

    glUniform3fv(glGetUniformLocation(mortonShader->ID, "sceneMin"), 1, glm::value_ptr(globalAABB.min));
    extent = glm::max(extent, glm::vec3(0.0001f)); 
    glUniform3fv(glGetUniformLocation(mortonShader->ID, "sceneExtent"), 1, glm::value_ptr(extent));
    glUniform1ui(glGetUniformLocation(mortonShader->ID, "numElements"), numTris);

    uint32_t workGroups = (numTris + 255) / 256;

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);
    glDispatchCompute(workGroups, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);
    glFinish();

    MyglobalLogger().logMessage(Logger::INFO, "Morton codes computed successfully", __FILE__, __LINE__);
    std::vector<MortonCodeElement> mortonData(numTris);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, mortonBuffer);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, numTris * sizeof(MortonCodeElement), mortonData.data());

    if (mortonData.size() >= 3) {
        MyglobalLogger().logMessage(Logger::DEBUG,
            "First Morton codes: [0]=" + std::to_string(mortonData[0].mortonCode) +
            " [1]=" + std::to_string(mortonData[1].mortonCode) +
            " [2]=" + std::to_string(mortonData[2].mortonCode), __FILE__, __LINE__);
    }

    std::sort(mortonData.begin(), mortonData.end(), [](const MortonCodeElement& a, const MortonCodeElement& b) {
        if (a.mortonCode == b.mortonCode) {
            return a.elementIdx < b.elementIdx; 
        }
        return a.mortonCode < b.mortonCode;
        });

    MyglobalLogger().logMessage(Logger::INFO, "Morton codes sorted successfully", __FILE__, __LINE__);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, mortonBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, numTris * sizeof(MortonCodeElement), mortonData.data(), GL_STATIC_DRAW);

    GLuint lbvhBuffer, lbvhConstructionBuffer;
    glGenBuffers(1, &lbvhBuffer);
    glGenBuffers(1, &lbvhConstructionBuffer);

    struct LBVHNode {
        int32_t left, right;
        uint32_t primitiveIdx;
        float aabbMinX, aabbMinY, aabbMinZ;
        float aabbMaxX, aabbMaxY, aabbMaxZ;
    };

    struct LBVHConstructionInfo {
        uint32_t parent;
        int32_t visitationCount;
    };

    uint32_t totalNodes = 2 * numTris - 1;

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lbvhBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, totalNodes * sizeof(LBVHNode), nullptr, GL_DYNAMIC_COPY);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lbvhConstructionBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, totalNodes * sizeof(LBVHConstructionInfo), nullptr, GL_DYNAMIC_COPY);

    glUseProgram(hierarchyShader->ID);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mortonBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, elemBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, lbvhBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, lbvhConstructionBuffer);

    glUniform1ui(glGetUniformLocation(hierarchyShader->ID, "numElements"), numTris);
    glUniform1ui(glGetUniformLocation(hierarchyShader->ID, "absolutePointers"), 1);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);
    glDispatchCompute(workGroups, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);
    glFinish();

    MyglobalLogger().logMessage(Logger::INFO, "LBVH hierarchy constructed", __FILE__, __LINE__);

    glUseProgram(aabbShader->ID);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lbvhBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, lbvhConstructionBuffer);

    glUniform1ui(glGetUniformLocation(aabbShader->ID, "numElements"), numTris);
    glUniform1ui(glGetUniformLocation(aabbShader->ID, "absolutePointers"), 1);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);
    glDispatchCompute(workGroups, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);
    glFinish();

    MyglobalLogger().logMessage(Logger::INFO, "LBVH AABB computed", __FILE__, __LINE__);

    std::vector<LBVHNode> lbvhNodes(totalNodes);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lbvhBuffer);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, totalNodes * sizeof(LBVHNode), lbvhNodes.data());

    std::vector<glm::vec3> instanceData;
    uint32_t validNodeCount = 0;
    uint32_t degenerateCount = 0;

    for (uint32_t i = numTris - 1; i < totalNodes; ++i) {
        const LBVHNode& node = lbvhNodes[i];

        glm::vec3 nodeMin(node.aabbMinX, node.aabbMinY, node.aabbMinZ);
        glm::vec3 nodeMax(node.aabbMaxX, node.aabbMaxY, node.aabbMaxZ);
        glm::vec3 scale = nodeMax - nodeMin;

        // Проверяем валидность AABB
        if (glm::any(glm::lessThanEqual(scale, glm::vec3(0.0001f))) ||
            glm::any(glm::isnan(nodeMin)) || glm::any(glm::isnan(nodeMax)) ||
            glm::any(glm::isinf(nodeMin)) || glm::any(glm::isinf(nodeMax))) {
            degenerateCount++;
            if (degenerateCount <= 5) { 
                MyglobalLogger().logMessage(Logger::DEBUG,
                    "Skipping degenerate node " + std::to_string(i) +
                    " - min:(" + std::to_string(nodeMin.x) + "," + std::to_string(nodeMin.y) + "," + std::to_string(nodeMin.z) +
                    ") max:(" + std::to_string(nodeMax.x) + "," + std::to_string(nodeMax.y) + "," + std::to_string(nodeMax.z) +
                    ") scale:(" + std::to_string(scale.x) + "," + std::to_string(scale.y) + "," + std::to_string(scale.z) + ")",
                    __FILE__, __LINE__);
            }
            continue;
        }

        glm::vec3 center = (nodeMin + nodeMax) * 0.5f;
        center += glm::vec3(8.3, 0.0, 8.0);

        scale = glm::max(scale, glm::vec3(0.001f)); 

        instanceData.push_back(center);
        instanceData.push_back(scale);
        validNodeCount++;

        if (validNodeCount <= 5) {
            bool isLeaf = (i >= numTris - 1);
            std::string nodeType = isLeaf ? "Leaf" : "Internal";
            MyglobalLogger().logMessage(Logger::DEBUG,
                nodeType + " node " + std::to_string(i) + " -> instance " + std::to_string(validNodeCount - 1) +
                ": center=(" + std::to_string(center.x) + "," + std::to_string(center.y) + "," + std::to_string(center.z) +
                "), scale=(" + std::to_string(scale.x) + "," + std::to_string(scale.y) + "," + std::to_string(scale.z) + ")",
                __FILE__, __LINE__);
        }
    }

    numInternalNodes = validNodeCount;

    MyglobalLogger().logMessage(Logger::INFO,
        "LBVH processing complete: " + std::to_string(validNodeCount) + " valid nodes from " +
        std::to_string(totalNodes) + " total (" + std::to_string(degenerateCount) + " degenerate skipped)",
        __FILE__, __LINE__);

    if (numInternalNodes > 0) {

        if (aabbInstanceVBO != 0) {
            glDeleteBuffers(1, &aabbInstanceVBO);
            aabbInstanceVBO = 0;
        }

        glGenBuffers(1, &aabbInstanceVBO);
        glBindBuffer(GL_ARRAY_BUFFER, aabbInstanceVBO);
        glBufferData(GL_ARRAY_BUFFER, instanceData.size() * sizeof(glm::vec3), instanceData.data(), GL_STATIC_DRAW);

        GLint bufferSize;
        glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);
        GLint expectedSize = instanceData.size() * sizeof(glm::vec3);

        if (bufferSize != expectedSize) {
            MyglobalLogger().logMessage(Logger::ERROR,
                "VBO creation failed: expected " + std::to_string(expectedSize) +
                " bytes, got " + std::to_string(bufferSize), __FILE__, __LINE__);
        }
        else {
            MyglobalLogger().logMessage(Logger::INFO,
                "LBVH instance VBO created: ID=" + std::to_string(aabbInstanceVBO) +
                ", size=" + std::to_string(bufferSize) + " bytes, instances=" + std::to_string(numInternalNodes),
                __FILE__, __LINE__);
        }

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        if (instanceData.size() >= 6) {
            MyglobalLogger().logMessage(Logger::DEBUG,
                "First LBVH instance data: center=(" +
                std::to_string(instanceData[0].x) + "," + std::to_string(instanceData[0].y) + "," + std::to_string(instanceData[0].z) +
                "), scale=(" +
                std::to_string(instanceData[1].x) + "," + std::to_string(instanceData[1].y) + "," + std::to_string(instanceData[1].z) + ")",
                __FILE__, __LINE__);
        }
    }
    else {
        MyglobalLogger().logMessage(Logger::WARNING, "No valid LBVH nodes created - visualization will be empty!", __FILE__, __LINE__);
        numInternalNodes = 0;
        if (aabbInstanceVBO != 0) {
            glDeleteBuffers(1, &aabbInstanceVBO);
            aabbInstanceVBO = 0;
        }
    }

    glDeleteBuffers(1, &elemBuffer);
    glDeleteBuffers(1, &mortonBuffer);
    glDeleteBuffers(1, &lbvhBuffer);
    glDeleteBuffers(1, &lbvhConstructionBuffer);

    // Проверяем ошибки OpenGL
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        MyglobalLogger().logMessage(Logger::ERROR,
            "OpenGL error after LBVH build: " + std::to_string(error), __FILE__, __LINE__);
    }

    MyglobalLogger().logMessage(Logger::INFO,
        "LBVH build completed successfully with " + std::to_string(numInternalNodes) + " visualization nodes",
        __FILE__, __LINE__);
}