#include "AABB.h"

bool AABB::inside(const glm::vec3& point) const {
    return (point.x >= minBounds.x && point.x <= maxBounds.x) &&
           (point.y >= minBounds.y && point.y <= maxBounds.y) &&
           (point.z >= minBounds.z && point.z <= maxBounds.z);
}

void AABB::extend(const glm::vec3& point) {
    minBounds = glm::min(minBounds, point);
    maxBounds = glm::max(maxBounds, point);
}

void AABB::extend(const AABB& box) {
    minBounds = glm::min(minBounds, box.minBounds);
    maxBounds = glm::max(maxBounds, box.maxBounds);
}

