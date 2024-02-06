#include "BVH.h"
#include "primitive/mesh.h"
#include <iostream>

// Construct BVH class
BVH::BVH(const std::vector<RenderShapeData>& shapes) : originalShapes(shapes) {
    std::vector<int> indices(originalShapes.size()); //  Create a indices list of shapes for tracking and comparison
    std::iota(indices.begin(), indices.end(), 0);  // initialize indices
    root = build(indices); // Build the tree
}

BVH::BVH(const Mesh& mesh) {
    std::vector<int> indices(mesh.faces.size()); // Indices list of triangles for tracking
    std::iota(indices.begin(), indices.end(), 0);  // initialize indices
    root = buildForMesh(mesh, indices); // Build the tree for the mesh
}

// Destroyer of BVH class
BVH::~BVH() {
    deleteNode(root);
}

/******************************** Functions to build BVH ********************************/
// Build the BVH recursively
BVHNode* BVH::build(std::vector<int>& indices) {
    // Base case: If only one shape, create a leaf node.
    if (indices.size() == 1) {
        BVHNode* leaf = new BVHNode();
        leaf->shapeIndex = indices[0];
        leaf->bounds = computeAABBForShape(originalShapes[indices[0]]);
        return leaf;
    }

    // Compute the bounding box for all shapes within the indices
    AABB combinedBox;
    for (int index : indices) {
        combinedBox.extend(computeAABBForShape(originalShapes[index]));
    }

    // Decide which axix to be cut off
    // Note: Cut off along the longest axis of the bounding box
    glm::vec3 dimensions = combinedBox.maxBounds - combinedBox.minBounds;
    int axis;
    if (dimensions.x >= dimensions.y && dimensions.x >= dimensions.z) {
        axis = 0;  // x-axis
    } else if (dimensions.y >= dimensions.x && dimensions.y >= dimensions.z) {
        axis = 1;  // y-axis
    } else {
        axis = 2;  // z-axis
    }

    // Sort indices based on their centroid in the chosen axis
    // Note: This is a lambda expression which defines a customed operator to utilize std::sort
    std::sort(indices.begin(), indices.end(), [this, axis](int a, int b) {
        glm::vec3 centroidA = (computeAABBForShape(originalShapes[a]).minBounds + computeAABBForShape(originalShapes[a]).maxBounds) * 0.5f;
        glm::vec3 centroidB = (computeAABBForShape(originalShapes[b]).minBounds + computeAABBForShape(originalShapes[b]).maxBounds) * 0.5f;
        return centroidA[axis] < centroidB[axis];
    });

    // Divide the indices to left and right
    size_t midPoint = indices.size() / 2;
    std::vector<int> leftIndices(indices.begin(), indices.begin() + midPoint);
    std::vector<int> rightIndices(indices.begin() + midPoint, indices.end());

    // Create an intermediate node, assign the combined BBox and do the recursion
    BVHNode* node = new BVHNode();
    node->bounds = combinedBox;
    node->left = build(leftIndices);
    node->right = build(rightIndices);

    return node;
}

BVHNode* BVH::buildForMesh(const Mesh& mesh, std::vector<int>& indices) {
    // Base case: If only one triangle, create a leaf node.
    if (indices.size() == 1) {
        BVHNode* leaf = new BVHNode();
        leaf->triangleIndex = indices[0];  // set triangleIndex instead of shapeIndex
        leaf->bounds = computeAABBForTriangle(mesh, indices[0]);
        return leaf;
    }

    // Compute the bounding box for all triangles within the indices
    AABB combinedBox;
    for (int index : indices) {
        combinedBox.extend(computeAABBForTriangle(mesh, index));
    }

    // Decide which axis to be cut off
    glm::vec3 dimensions = combinedBox.maxBounds - combinedBox.minBounds;
    int axis;
    if (dimensions.x >= dimensions.y && dimensions.x >= dimensions.z) {
        axis = 0;  // x-axis
    } else if (dimensions.y >= dimensions.x && dimensions.y >= dimensions.z) {
        axis = 1;  // y-axis
    } else {
        axis = 2;  // z-axis
    }

    // Sort indices based on the triangle centroid in the chosen axis
    std::sort(indices.begin(), indices.end(), [&mesh, axis, this](int a, int b) {
        AABB aabbA = computeAABBForTriangle(mesh, a);
        AABB aabbB = computeAABBForTriangle(mesh, b);

        glm::vec3 centroidA = (aabbA.minBounds + aabbA.maxBounds) * 0.5f;
        glm::vec3 centroidB = (aabbB.minBounds + aabbB.maxBounds) * 0.5f;

        return centroidA[axis] < centroidB[axis];
    });

    // Divide the indices to left and right
    size_t midPoint = indices.size() / 2;
    std::vector<int> leftIndices(indices.begin(), indices.begin() + midPoint);
    std::vector<int> rightIndices(indices.begin() + midPoint, indices.end());

    // Create an intermediate node, assign the combined BBox and do the recursion
    BVHNode* node = new BVHNode();
    node->bounds = combinedBox;
    node->left = buildForMesh(mesh, leftIndices);
    node->right = buildForMesh(mesh, rightIndices);

    return node;
}


// Compute the CTM transformed AABB for a shape
AABB BVH::computeAABBForShape(const RenderShapeData& shape) {
    AABB localAABB;

    // Set the local AABB based on the primitive type
    // Note: non-mesh primitives are all bounded in [-0.5, 0.5]
    if (shape.primitive.type == PrimitiveType::PRIMITIVE_MESH) {
        // Load the mesh from caching to avoid repetitive loading)
        Mesh mesh = MeshCache::getInstance().loadMeshWithCache(shape.primitive.meshfile);

        // Compute AABB for the entire mesh
        for (const auto& vertex : mesh.vertices) {
            localAABB.extend(vertex);
        }
    } else {
        // Existing logic for non-mesh primitives
        localAABB.minBounds = glm::vec3(-0.5f, -0.5f, -0.5f);
        localAABB.maxBounds = glm::vec3(0.5f, 0.5f, 0.5f);
    }

    // Transform each corner of the local AABB by the shape's CTM
    // Note: should have 8 points for the bounding box
    std::vector<glm::vec3> transformedCorners;
    for (float x : {localAABB.minBounds.x, localAABB.maxBounds.x}) {
        for (float y : {localAABB.minBounds.y, localAABB.maxBounds.y}) {
            for (float z : {localAABB.minBounds.z, localAABB.maxBounds.z}) {
                glm::vec4 transformed = shape.ctm * glm::vec4(x, y, z, 1.0f);
                transformedCorners.push_back(glm::vec3(transformed.x, transformed.y, transformed.z));
            }
        }
    }

    // Compute the world space AABB from the 8 transformed corners
    AABB worldAABB;
    for (const auto& corner : transformedCorners) {
        worldAABB.extend(corner);
    }

    return worldAABB;
}

AABB BVH::computeAABBForTriangle(const Mesh& mesh, int triangleIndex) {
    AABB triangleAABB;

    Face triangle = mesh.faces[triangleIndex];

    triangleAABB.extend(mesh.vertices[triangle.v[0]]);
    triangleAABB.extend(mesh.vertices[triangle.v[1]]);
    triangleAABB.extend(mesh.vertices[triangle.v[2]]);

    return triangleAABB;
}

/******************************** Functions to traverse BVH ********************************/
// Get intersected nodes
std::vector<RenderShapeData> BVH::potentialIntersections(const glm::vec4& cameraPos, const glm::vec4& d) const {
    std::vector<RenderShapeData> potentialShapes;
    potentialIntersectionsRecursive(root, cameraPos, d, potentialShapes);
    return potentialShapes;
}

// Traverse in the BVH and add intersected nodes into potentialShapes
void BVH::potentialIntersectionsRecursive(BVHNode* node, const glm::vec4& cameraPos, const glm::vec4& d, std::vector<RenderShapeData>& potentialShapes) const {
    if (!node) {
        return;
    }

    if (intersects(node->bounds, cameraPos, d)) {
        if (node->shapeIndex >= 0) {
            potentialShapes.push_back(originalShapes[node->shapeIndex]);
        } else {
            potentialIntersectionsRecursive(node->left, cameraPos, d, potentialShapes);
            potentialIntersectionsRecursive(node->right, cameraPos, d, potentialShapes);
        }
    }
}

// Get intersected triangle indices
std::vector<int> BVH::potentialIntersectionsForMesh(const glm::vec4& cameraPos, const glm::vec4& d) const {
    std::vector<int> potentialTriangles;
    potentialIntersectionsRecursiveForMesh(root, cameraPos, d, potentialTriangles);
    return potentialTriangles;
}

// Traverse in the BVH and add intersected triangle indices into potentialTriangles
void BVH::potentialIntersectionsRecursiveForMesh(BVHNode* node, const glm::vec4& cameraPos, const glm::vec4& d, std::vector<int>& potentialTriangles) const {
    if (!node) {
        return;
    }

    if (intersects(node->bounds, cameraPos, d)) {
        if (node->triangleIndex >= 0) {
            potentialTriangles.push_back(node->triangleIndex);
        } else {
            potentialIntersectionsRecursiveForMesh(node->left, cameraPos, d, potentialTriangles);
            potentialIntersectionsRecursiveForMesh(node->right, cameraPos, d, potentialTriangles);
        }
    }
}

// Check whether a ray intersects AABB
bool BVH::intersects(const AABB& box, const glm::vec4& cameraPos, const glm::vec4& d) const {
    float tmin = (box.minBounds.x - cameraPos.x) / d.x;
    float tmax = (box.maxBounds.x - cameraPos.x) / d.x;

    if (tmin > tmax) std::swap(tmin, tmax);

    float tymin = (box.minBounds.y - cameraPos.y) / d.y;
    float tymax = (box.maxBounds.y - cameraPos.y) / d.y;

    if (tymin > tymax) std::swap(tymin, tymax);

    if ((tmin > tymax) || (tymin > tmax)) return false;

    if (tymin > tmin) tmin = tymin;
    if (tymax < tmax) tmax = tymax;

    float tzmin = (box.minBounds.z - cameraPos.z) / d.z;
    float tzmax = (box.maxBounds.z - cameraPos.z) / d.z;

    if (tzmin > tzmax) std::swap(tzmin, tzmax);

    if ((tmin > tzmax) || (tzmin > tmax)) return false;

    return true;
}

void BVH::deleteNode(BVHNode* node) {
    if (node) {
        deleteNode(node->left);
        deleteNode(node->right);
        delete node;
    }
}
