#pragma once

#include <glm/glm.hpp>

struct AABB {
    glm::vec3 minBounds;
    glm::vec3 maxBounds;

    // Default constructor
    AABB() : minBounds(glm::vec3(FLT_MAX)), maxBounds(glm::vec3(-FLT_MAX)) {}

    // Check if a point is inside the bounding box
    bool inside(const glm::vec3& point) const;

    // Extend the bounding box to include a point
    void extend(const glm::vec3& point);

    // Extend the bounding box to include another bounding box
    void extend(const AABB& box);
};
