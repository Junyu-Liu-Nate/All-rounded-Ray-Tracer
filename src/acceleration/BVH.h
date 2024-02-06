#pragma once

#include "BVHNode.h"
#include "utils/sceneparser.h"
#include "primitive/meshcache.h"

#include <vector>
#include <numeric>


class BVH {
public:
    BVH(const std::vector<RenderShapeData>& shapes);
    BVH(const Mesh& mesh);
    ~BVH();
    std::vector<RenderShapeData> potentialIntersections(const glm::vec4& cameraPos, const glm::vec4& d) const;
    bool intersects(const AABB& box, const glm::vec4& cameraPos, const glm::vec4& d) const;
    std::vector<int> potentialIntersectionsForMesh(const glm::vec4& cameraPos, const glm::vec4& d) const;

private:
    BVHNode* root;
    std::vector<RenderShapeData> originalShapes;

    BVHNode* build(std::vector<int>& indices);
    BVHNode* buildForMesh(const Mesh& mesh, std::vector<int>& indices);
    void deleteNode(BVHNode* node);
    AABB computeAABBForShape(const RenderShapeData& shape);
    AABB computeAABBForTriangle(const Mesh& mesh, int triangleIndex);
    void potentialIntersectionsRecursive(BVHNode* node, const glm::vec4& cameraPos, const glm::vec4& d, std::vector<RenderShapeData>& potentialShapes) const;
    void potentialIntersectionsRecursiveForMesh(BVHNode* node, const glm::vec4& cameraPos, const glm::vec4& d, std::vector<int>& potentialTriangles) const;
};
