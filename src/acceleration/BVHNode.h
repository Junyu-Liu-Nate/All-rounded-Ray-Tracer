#pragma once

#include "AABB.h"

struct BVHNode {
    AABB bounds;
    BVHNode* left;
    BVHNode* right;

    int shapeIndex;  // -1 for internal nodes
    int triangleIndex = -1;

    BVHNode() : left(nullptr), right(nullptr), shapeIndex(-1) {}
};
