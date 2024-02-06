#pragma once

#include <glm/glm.hpp>
#include <QQueue>
#include <QPair>
#include <QMutex>
#include <QMutexLocker>
#include "utils/rgba.h"
#include "primitive/primitivefunction.h"
#include "acceleration/BVH.h"
#include "antialias/filter.h"
#include "primitive/mesh.h"
#include "primitive/meshcache.h"

// A forward declaration for the RaytraceScene class
class RayTraceScene;

// These will allow access to the global task queue and mutex from other parts of the code.
extern QQueue<QPair<int, int>> taskQueue;
extern QMutex taskQueueMutex;

// A class representing a ray-tracer
class RayTracer
{
public:
    struct Config {
        bool enableShadow        = false;
        bool enableReflection    = false;
        bool enableRefraction    = false;
        bool enableTextureMap    = false;
        bool enableTextureFilter = false;
        bool enableParallelism   = false;
        bool enableSuperSample   = false;
        bool enableAcceleration  = false;
        bool enableDepthOfField  = false;
        int maxRecursiveDepth    = 4;
        bool onlyRenderNormals   = false;
        bool enableSoftShadows    = true;
    };

public:
    RayTracer(Config config);
    BVH* m_bvh;

    // Renders the scene synchronously.
    // The ray-tracer will render the scene and fill imageData in-place.
    // @param imageData The pointer to the imageData to be filled.
    // @param scene The scene to be rendered.
    void render(RGBA *imageData, RayTraceScene &scene);

private:
    const Config m_config;

    void renderSegment(RGBA* imageData, const RayTraceScene& scene, int startRow, int endRow);
    void renderBlock(RGBA* imageData, const RayTraceScene& scene, int startX, int startY, int endX, int endY);

//    glm::vec3 computeRayColor(const RayTraceScene& scene, float i, float j);
    glm::vec4 computeRayColor(const RayTraceScene& scene, std::vector<glm::vec4> ray, int recursionDepth);
    std::vector<glm::vec4> calculateRayInfo(const RayTraceScene& scene, float i, float j);
    bool calculateIntersection(std::vector<RenderShapeData> &shapes, glm::vec4 &cameraPos, glm::vec4 &d, float &t, glm::vec3 &normal, RenderShapeData &intersectShape);
    void calculateLighting(const RayTraceScene &scene, glm::vec4 &cameraPos, glm::vec4 &d, float &t, glm::vec3 &normal, RenderShapeData &intersectShape, glm::vec4 &illumination);
    void mapNormalColor(glm::vec3 inColor, glm::vec3 &outColor);
    void mapIlluminationColor(glm::vec3 inColor, glm::vec3 &outColor);

    std::vector<RGBA> loadImage(const QString &filePath, int &width, int &height);

    glm::vec4 refractDirection(const glm::vec4& d, const glm::vec3& normal, float ior);
};


