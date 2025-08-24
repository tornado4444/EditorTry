#ifndef LBVH_COMMON_GLSL
#define LBVH_COMMON_GLSL

#define INVALID_POINTER 0xFFFFFFFFu

struct Element {
    uint primitiveIdx;
    float aabbMinX, aabbMinY, aabbMinZ;
    float aabbMaxX, aabbMaxY, aabbMaxZ;
};

struct LBVHNode {
    int left; 
    int right; 
    uint primitiveIdx; 
    float aabbMinX, aabbMinY, aabbMinZ;
    float aabbMaxX, aabbMaxY, aabbMaxZ;
};

struct MortonCodeElement {
    uint mortonCode;
    uint elementIdx;
};

struct LBVHConstructionInfo {
    uint parent;
    int visitationCount;
};

#endif