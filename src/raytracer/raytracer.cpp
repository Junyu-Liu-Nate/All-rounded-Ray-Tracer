#include "raytracer.h"
#include "raytracescene.h"
#include <iostream>
#include <QThreadPool>
#include <QFuture>
#include <QtConcurrent>

QQueue<QPair<int, int>> taskQueue;
QMutex taskQueueMutex;

RayTracer::RayTracer(Config config) :
    m_config(config)
{}

using TNormalTuple = std::tuple<float, glm::vec3>;

// Main function to be called for render
void RayTracer::render(RGBA *imageData, RayTraceScene &scene) {
    // Create bvh for each mesh in the shape list
    for (auto &shape : scene.sceneMetaData.shapes) {
        if (shape.primitive.type == PrimitiveType::PRIMITIVE_MESH) {
            Mesh mesh = MeshCache::getInstance().loadMeshWithCache(shape.primitive.meshfile);
            shape.triangleBVH = std::make_shared<BVH>(mesh);
        }
    }

    // Build BVH for shapes (if accelaration activated)
    if (m_config.enableAcceleration) {
        std::vector<RenderShapeData> sceneShapes = scene.sceneMetaData.shapes;
        m_bvh = new BVH(sceneShapes);
    }

    // Render image by dynamically render blocks or render the whole image
    if (m_config.enableParallelism) {
        // Dynamically determine the block size based on the number of processor cores
        const int numCores = QThread::idealThreadCount();
        const int BLOCK_SIZE = std::max(scene.width() / numCores, 32);

        // Declared taskQueue
        QQueue<QPair<QPair<int, int>, QPair<int, int>>> taskQueue;

        // Populate the global task queue with tasks of block size
        for (int y = 0; y < scene.height(); y += BLOCK_SIZE) {
            for (int x = 0; x < scene.width(); x += BLOCK_SIZE) {
                //  Note: Protect code (setting sizes and create queues) in this block by the mutex
                int endX = std::min(x + BLOCK_SIZE, scene.width());
                int endY = std::min(y + BLOCK_SIZE, scene.height());

                QPair<int, int> start = qMakePair(x, y);
                QPair<int, int> end = qMakePair(endX, endY);
                QPair<QPair<int, int>, QPair<int, int>> block = qMakePair(start, end);

                // Center prioritization
                int centerY = scene.height() / 2;
                int centerX = scene.width() / 2;

                if (y <= centerY && centerY < endY && x <= centerX && centerX < endX) {
                    QMutexLocker locker(&taskQueueMutex);
                    taskQueue.prepend(block);
                } else {
                    QMutexLocker locker(&taskQueueMutex);
                    taskQueue.enqueue(block);
                }
                // Note: Once the block ends, locker's destructor is called, and the mutex is unlocked.
            }
        }

        // Dynamic Task Fetching using QtConcurrent
        // Note: Create run logic within a lambda expression.
        //       Call run and pass the run logic to it to start multi-thread excecution.
        QFuture<void> future = QtConcurrent::run([&](){
            while (true) {
                taskQueueMutex.lock();
                if (taskQueue.isEmpty()) {
                    taskQueueMutex.unlock();
                    break;
                }
                auto block = taskQueue.dequeue();
                taskQueueMutex.unlock();

                renderBlock(imageData, scene, block.first.first, block.first.second, block.second.first, block.second.second);
            }
        });

        // Note: Main thread continue the create watcher and monitor the process.
        QFutureWatcher<void> watcher;
        watcher.setFuture(future);
        watcher.waitForFinished(); // Block the main thread and wait for all tasks to finish
    }
    else {
        renderBlock(imageData, scene, 0, 0, scene.width(), scene.height());
    }

    // Post-filtering for anti-aliasing
    filter postFilter;
    postFilter.bilateral2D(imageData, scene.width(), scene.height(), 10);
}

// Render a block on the image
void RayTracer::renderBlock(RGBA* imageData, const RayTraceScene& scene, int startX, int startY, int endX, int endY) {
    int planeW = scene.width();

    // Iterate on the pixels of a render block
    for (int j = startY; j < endY; j++) {
        for (int i = startX; i < endX; i++) {
            glm::vec4 illumination;
            glm::vec3 finalColor = glm::vec3(0);
            // Adaptive Super-sample (if super-sample is enabled)
            if (m_config.enableSuperSample) {
                std::vector<glm::vec4> samples;

                // Initial 4 samples (corners of the pixel)
                samples.push_back(computeRayColor(scene, calculateRayInfo(scene, i, j), m_config.maxRecursiveDepth));
                samples.push_back(computeRayColor(scene, calculateRayInfo(scene, i+1, j), m_config.maxRecursiveDepth));
                samples.push_back(computeRayColor(scene, calculateRayInfo(scene, i, j+1), m_config.maxRecursiveDepth));
                samples.push_back(computeRayColor(scene, calculateRayInfo(scene, i+1, j+1), m_config.maxRecursiveDepth));

                // Check variance
                glm::vec4 avgColor = (samples[0] + samples[1] + samples[2] + samples[3]) / 4.0f;
                float variance = 0.0f;
                for (const auto& color : samples) {
                    variance += glm::length(color - avgColor);
                }

                const float threshold = 0.1f;  // adjust as needed
                if (variance > threshold) {
                    // Add more samples
                    samples.push_back(computeRayColor(scene, calculateRayInfo(scene, i+0.5, j), 0));
                    samples.push_back(computeRayColor(scene, calculateRayInfo(scene, i, j+0.5), 0));
                    samples.push_back(computeRayColor(scene, calculateRayInfo(scene, i+0.5, j+0.5), 0));
                    samples.push_back(computeRayColor(scene, calculateRayInfo(scene, i+1, j+0.5), 0));
                    samples.push_back(computeRayColor(scene, calculateRayInfo(scene, i+0.5, j+1), 0));
                }

                // Average the colors
                for (const auto& color : samples) {
                    illumination += color;
                }
                illumination /= static_cast<float>(samples.size());
            }
            else {
                if (m_config.enableDepthOfField) {
                    const int SAMPLES_PER_AXIS = 3; // 3x3 super-sampling
                    const float SAMPLE_STEP = 1.0f / SAMPLES_PER_AXIS;

                    // Super-sampling
                    for (int sx = 0; sx < SAMPLES_PER_AXIS; sx++) {
                        for (int sy = 0; sy < SAMPLES_PER_AXIS; sy++) {
                            float si = i + sx * SAMPLE_STEP;
                            float sj = j + sy * SAMPLE_STEP;
                            illumination += computeRayColor(scene, calculateRayInfo(scene, si, sj), 0);
                        }
                    }
                    illumination /= (SAMPLES_PER_AXIS * SAMPLES_PER_AXIS); // Average the sampled colors
                }
                else {
                    illumination = computeRayColor(scene, calculateRayInfo(scene, i, j), 0);
                }
            }

            // Convert to [0,255] to get the final color
            if (m_config.onlyRenderNormals) {
                mapNormalColor(illumination.xyz(), finalColor);
            }
            else {
                mapIlluminationColor(illumination.xyz(), finalColor);
            }

            imageData[i + j * planeW].r = static_cast<uint8_t>(finalColor.r);
            imageData[i + j * planeW].g = static_cast<uint8_t>(finalColor.g);
            imageData[i + j * planeW].b = static_cast<uint8_t>(finalColor.b);
        }
    }
}

// Calculate the ray info that is shooting from camera
std::vector<glm::vec4> RayTracer::calculateRayInfo(const RayTraceScene& scene, float i, float j) {
    std::vector<glm::vec4> ray;

    // Get camera and ray info
    glm::vec4 cameraPos = scene.getCamera().cameraPos;
    glm::mat4 viewMatrix = scene.getCamera().getViewMatrix();
    glm::mat4 viewMatrixInverse = scene.getCamera().getViewMatrixInverse();
    float cameraHeightAngle = scene.getCamera().getHeightAngle();
    int planeH = scene.height();
    int planeW = scene.width();
    float k = 1;

    // Compute d
    float x = (i + 0.5) / planeW - 0.5;
    float y = (planeH - 1 - j + 0.5) / planeH - 0.5;
    float V = 2 * k * tan(cameraHeightAngle / 2);
    float U = V * scene.getCamera().getAspectRatio();
    float u = U * x;
    float v = V * y;

    glm::vec4 uvk(u, v, -k, 1);
    glm::vec4 d;

    if(m_config.enableDepthOfField) {
        glm::vec4 eye = viewMatrix*cameraPos; // In Camera Space
        glm::vec4 dCameraSpace = uvk - eye; // In Camera Space

        // Compute lens radius and lens offset
        float lensRadius = scene.getCamera().getAperture() / 2;
//        float lensRadius = scene.getCamera().getAperture();
//        float r = lensRadius * sqrt(static_cast<float>(rand()) / RAND_MAX);
        float r = lensRadius * 2;
        float theta = 2 * M_PI * (static_cast<float>(rand()) / RAND_MAX);
        glm::vec4 lensOffset(r * cos(theta), r * sin(theta), 0, 0);

        // Adjust ray direction for depth of field
        float focalDistance = scene.getCamera().getFocalLength();
        glm::vec4 focusPoint = eye + focalDistance * dCameraSpace;
        dCameraSpace = focusPoint - (eye + lensOffset);
        eye += lensOffset;

        d = viewMatrixInverse * dCameraSpace; // To World Space

        ray.push_back(viewMatrixInverse * eye);
        ray.push_back(d);

        return ray;
    }
    else {
        d = viewMatrixInverse * (uvk - viewMatrix*cameraPos); // Ray to World Space

        ray.push_back(cameraPos);
        ray.push_back(d);

        return ray;
    }
}


/************************** Functions for computing ray intersect colors **************************/
glm::vec4 RayTracer::computeRayColor(const RayTraceScene& scene, std::vector<glm::vec4> ray, int recursionDepth) {
    glm::vec4 cameraPos = ray.at(0);
    glm::vec4 d = ray.at(1);

    // Calculate intersections
    float t = 1000; // used to track the nearest intersection
    glm::vec3 normal{-1, -1, -1};
    RenderShapeData intersectShape; // The intersected shape that used to calculate lighting
    bool isIntersect;
    if (m_config.enableAcceleration) {
        std::vector<RenderShapeData> potentialIntersectShapes = m_bvh->potentialIntersections(cameraPos, d); // BVH version
        isIntersect = calculateIntersection(potentialIntersectShapes, cameraPos, d, t, normal,intersectShape);
    }
    else {
        std::vector<RenderShapeData> potentialIntersectShapes = scene.sceneMetaData.shapes;
        isIntersect = calculateIntersection(potentialIntersectShapes, cameraPos, d, t, normal,intersectShape);
    }

    // Calculate lighting (if intersect with some shape)
    glm::vec4 illumination(0, 0, 0, 1);
    if (isIntersect) {
        calculateLighting(scene, cameraPos, d, t, normal, intersectShape, illumination);

        // Reflection
        if (m_config.enableReflection && glm::length(intersectShape.primitive.material.cReflective) == 0) {
            glm::vec4 intersectPos = cameraPos + t * d;
            d = glm::normalize(d);
            normal = glm::normalize(normal);
            glm::vec4 reflectDirection = d - 2.0f * glm::dot(glm::vec4(normal, 0.0f), d) * glm::vec4(normal, 0.0f);
            ray.at(0) = intersectPos;
            ray.at(1) = reflectDirection;
            if (m_config.enableReflection && recursionDepth < m_config.maxRecursiveDepth) {
                illumination.x += scene.sceneMetaData.globalData.ks * intersectShape.primitive.material.cReflective.x * computeRayColor(scene, ray, recursionDepth+1).x;
                illumination.y += scene.sceneMetaData.globalData.ks * intersectShape.primitive.material.cReflective.y * computeRayColor(scene, ray, recursionDepth+1).y;
                illumination.z += scene.sceneMetaData.globalData.ks * intersectShape.primitive.material.cReflective.z * computeRayColor(scene, ray, recursionDepth+1).z;
                illumination.w = 1;
            }
        }

        // Refraction
        if (m_config.enableRefraction) {
            TNormalTuple intersectTuple;
            PrimitiveFunction pf;
            float intersectIn;
            glm::vec3 normalIn;

            glm::vec4 intersectPosIn = cameraPos + (t + 0.01f) * d;
            d = glm::normalize(d);
            normal = glm::normalize(normal);
            glm::vec4 refractDirectionIn = refractDirection(d, normal, intersectShape.primitive.material.ior);

            glm::mat4 ctm = intersectShape.ctm;
            glm::vec4 pObjectSpace = glm::inverse(ctm) * intersectPosIn; // Ray to Object Space
            glm::vec4 dObjectSpace = glm::inverse(ctm) * refractDirectionIn; // Ray to Object Space
            glm::mat3 upperLeft33(ctm[0].x, ctm[0].y, ctm[0].z,
                                  ctm[1].x, ctm[1].y, ctm[1].z,
                                  ctm[2].x, ctm[2].y, ctm[2].z);
            if (intersectShape.primitive.type == PrimitiveType::PRIMITIVE_SPHERE) {
                intersectTuple = pf.sphereIntersectInside(pObjectSpace, dObjectSpace);
            }
            if (intersectShape.primitive.type == PrimitiveType::PRIMITIVE_CUBE) {
                intersectTuple = pf.cubeIntersectFromInside(pObjectSpace, dObjectSpace);
            }
            if (intersectShape.primitive.type == PrimitiveType::PRIMITIVE_CYLINDER) {
                intersectTuple = pf.cylinderIntersectInside(pObjectSpace, dObjectSpace);
            }
            if (intersectShape.primitive.type == PrimitiveType::PRIMITIVE_CONE) {
                intersectTuple = pf.coneIntersectInside(pObjectSpace, dObjectSpace);
            }

            intersectIn = std::get<0>(intersectTuple);
            if (intersectIn > 0) {
                glm::vec3 normalInObjectSpace = std::get<1>(intersectTuple);
                normalIn = glm::inverse(glm::transpose(upperLeft33)) * normalInObjectSpace; // Normal to World Space
                normalIn = normalIn / glm::length(normalIn);
            }

            glm::vec4 intersectPosOut = intersectPosIn + intersectIn * refractDirectionIn;
            glm::vec4 refractDirectionOut = refractDirection(refractDirectionIn, normalIn, intersectShape.primitive.material.ior);

            ray.at(0) = intersectPosOut;
            ray.at(1) = refractDirectionOut;
            if (m_config.enableRefraction && recursionDepth < m_config.maxRecursiveDepth) {
                illumination.x += scene.sceneMetaData.globalData.kt * intersectShape.primitive.material.cTransparent.x * computeRayColor(scene, ray, recursionDepth+1).x;
                illumination.y += scene.sceneMetaData.globalData.kt * intersectShape.primitive.material.cTransparent.y * computeRayColor(scene, ray, recursionDepth+1).y;
                illumination.z += scene.sceneMetaData.globalData.kt * intersectShape.primitive.material.cTransparent.z * computeRayColor(scene, ray, recursionDepth+1).z;
                illumination.w = 1;
            }
        }
    }

    if (m_config.onlyRenderNormals) {
        return glm::vec4(normal, 1);
    }
    else {
        return illumination;
    }
}

// Calculate the direction of refracted ray
glm::vec4 RayTracer::refractDirection(const glm::vec4& d, const glm::vec3& normal, float ior) {
    // Determine if the ray is inside the medium by checking the angle with the normal
    bool isInside = glm::dot(d, glm::vec4(normal, 0.0f)) < 0.0f;

    // If inside the medium, the normal should be reversed and the ior should be inverted
    glm::vec3 adjustedNormal = isInside ? -normal : normal;
    float eta = isInside ? ior : 1.0f / ior;

    float cosThetaI = glm::clamp(-1.0f, 1.0f, glm::dot(d, glm::vec4(adjustedNormal, 0.0f)));
    float sinThetaI = glm::sqrt(1.0f - cosThetaI * cosThetaI);
    float sinThetaT = eta * sinThetaI;

    // Handle total internal reflection
    if (sinThetaT > 1.0f) {
        return glm::vec4(0.0f); // No refraction, return a zero vector
    }

    float cosThetaT = glm::sqrt(1.0f - sinThetaT * sinThetaT);

    return eta * d + (eta * cosThetaI - cosThetaT) * glm::vec4(adjustedNormal, 0.0f);
}


bool RayTracer::calculateIntersection(std::vector<RenderShapeData> &shapes, glm::vec4 &cameraPos, glm::vec4 &d, float &t, glm::vec3 &normal, RenderShapeData &intersectShape) {
    TNormalTuple intersectTuple;
    PrimitiveFunction pf;
    float nearestIntersect;

    bool isIntersect = false;
    for (RenderShapeData &shape : shapes) {
        glm::mat4 ctm = shape.ctm;
        glm::vec4 pObjectSpace = glm::inverse(ctm) * cameraPos; // Ray to Object Space
        glm::vec4 dObjectSpace = glm::inverse(ctm) * d;         // Ray to Object Space
        glm::mat3 upperLeft33(ctm[0].x, ctm[0].y, ctm[0].z,
                              ctm[1].x, ctm[1].y, ctm[1].z,
                              ctm[2].x, ctm[2].y, ctm[2].z);
        switch (shape.primitive.type) {
            case PrimitiveType::PRIMITIVE_CUBE:
                intersectTuple = pf.cubeIntersect(pObjectSpace, dObjectSpace);
                break;

            case PrimitiveType::PRIMITIVE_CONE:
                intersectTuple = pf.coneIntersect(pObjectSpace, dObjectSpace);
                break;

            case PrimitiveType::PRIMITIVE_CYLINDER:
                intersectTuple = pf.cylinderIntersect(pObjectSpace, dObjectSpace);
                break;

            case PrimitiveType::PRIMITIVE_SPHERE:
                intersectTuple = pf.sphereIntersect(pObjectSpace, dObjectSpace);
                break;

            case PrimitiveType::PRIMITIVE_MESH:
                // Find potential triangles the ray might intersect using the BVH
                std::vector<int> potentialTriangleIndices = shape.triangleBVH->potentialIntersectionsForMesh(pObjectSpace, dObjectSpace);

                Mesh mesh = MeshCache::getInstance().loadMeshWithCache(shape.primitive.meshfile);

                float closestIntersection = 1000;
                TNormalTuple closestIntersectTuple;

                // Intersect ray with these potential triangles
                for (int triangleIndex : potentialTriangleIndices) {
                    Face face = mesh.faces[triangleIndex];

                    Triangle triangle;
                    triangle.v0 = mesh.vertices[face.v[0]];
                    triangle.v1 = mesh.vertices[face.v[1]];
                    triangle.v2 = mesh.vertices[face.v[2]];

                    TNormalTuple currentIntersectTuple = pf.triangleIntersect(pObjectSpace, dObjectSpace, triangle);

                    if (std::get<0>(currentIntersectTuple) > 0 && std::get<0>(currentIntersectTuple) < closestIntersection) {
                        closestIntersection = std::get<0>(currentIntersectTuple);
                        closestIntersectTuple = currentIntersectTuple;
                    }
                }

                intersectTuple = closestIntersectTuple;
                break;
        }
        nearestIntersect = std::get<0>(intersectTuple);
        if (nearestIntersect > 0 && nearestIntersect < t) {
            isIntersect = true;
            t = nearestIntersect;
            glm::vec3 normalObjectSpace = std::get<1>(intersectTuple);
            normal = glm::inverse(glm::transpose(upperLeft33)) * normalObjectSpace; // Normal to World Space
            normal = normal / glm::length(normal);

            intersectShape = shape;
        }
    }
    return isIntersect;
}

void RayTracer::calculateLighting(const RayTraceScene &scene, glm::vec4 &cameraPos, glm::vec4 &d, float &t, glm::vec3 &normal, RenderShapeData &intersectShape, glm::vec4 &illumination) {
    glm::vec3 directionToCamera = -d;
    directionToCamera = glm::normalize(directionToCamera);
    SceneMaterial material = intersectShape.primitive.material;

    // Check whether normal direction is pointing to camera
    float checkNormal = glm::dot(normal, directionToCamera);
    if (checkNormal < 0) {
        normal = -normal;
    }

    glm::vec4 intersectPos = cameraPos + t * d;

    // Ambient term
    illumination += scene.sceneMetaData.globalData.ka *  material.cAmbient;

    for (SceneLightData light : scene.sceneMetaData.lights) {
        glm::vec4 color = light.color;
        float distanceToLight;
        float att;
        glm::vec3 Li;
        float falloff = 0;

        switch (light.type) {
            case LightType::LIGHT_DIRECTIONAL:
                att = 1.0;
                Li = glm::normalize(-light.dir.xyz());
                break;

            case LightType::LIGHT_POINT:
                distanceToLight = glm::length(intersectPos - light.pos);
                att = std::min(1.0, 1.0 / (light.function.x + light.function.y * distanceToLight + light.function.z * distanceToLight * distanceToLight));
                Li = glm::normalize((light.pos - intersectPos).xyz());
                break;

            case LightType::LIGHT_SPOT:
                distanceToLight = glm::length(intersectPos - light.pos);
                att = std::min(1.0, 1.0 / (light.function.x + light.function.y * distanceToLight + light.function.z * distanceToLight * distanceToLight));
                Li = glm::normalize((light.pos - intersectPos).xyz());

                // Calculate fall off
                glm::vec3 l_out = glm::normalize(intersectPos - light.pos);
                glm::vec3 l_dir = glm::normalize(light.dir);
                float xAngle = glm::acos(glm::clamp(glm::dot(l_out,l_dir), 0.0f, 1.0f));
                float innerAngle = light.angle - light.penumbra;
                if (xAngle > innerAngle && xAngle < light.angle) {
                    falloff = -2 * std::pow((xAngle - innerAngle) / light.penumbra, 3) + 3 * std::pow((xAngle - innerAngle) / light.penumbra, 2);
                }
                if (xAngle >= light.angle) {
                    falloff = 1;
                }
                break;
        }
        // Texture
        glm::vec3 textureColor = {0,0,0};
        if (m_config.enableTextureMap && material.textureMap.isUsed) {
            glm::mat4 ctm = intersectShape.ctm;
            glm::vec4 pObjectSpace = glm::inverse(ctm) * cameraPos; // Ray to Object Space
            glm::vec4 dObjectSpace = glm::inverse(ctm) * d;         // Ray to Object Space
            PrimitiveFunction pf;
            if (intersectShape.primitive.type == PrimitiveType::PRIMITIVE_SPHERE) {
                textureColor = pf.sphereTexture(m_config.enableTextureFilter, pObjectSpace, dObjectSpace, t, material.textureMap.repeatU, material.textureMap.repeatV, QString::fromStdString(material.textureMap.filename));
            }
            if (intersectShape.primitive.type == PrimitiveType::PRIMITIVE_CUBE) {
                textureColor = pf.cubeTexture(true, pObjectSpace, dObjectSpace, t, material.textureMap.repeatU, material.textureMap.repeatV, QString::fromStdString(material.textureMap.filename));
            }
            if (intersectShape.primitive.type == PrimitiveType::PRIMITIVE_CYLINDER) {
                textureColor = pf.cylinderTexture(true, pObjectSpace, dObjectSpace, t, material.textureMap.repeatU, material.textureMap.repeatV, QString::fromStdString(material.textureMap.filename));
            }
            if (intersectShape.primitive.type == PrimitiveType::PRIMITIVE_CONE) {
                textureColor = pf.coneTexture(m_config.enableTextureFilter, pObjectSpace, dObjectSpace, t, material.textureMap.repeatU, material.textureMap.repeatV, QString::fromStdString(material.textureMap.filename));
            }
        }

        // Diffuse term
        float diffuseDot = glm::dot(normal, Li);
        float diffuseClamped = glm::clamp(diffuseDot, 0.0f, 1.0f);
        glm::vec4 diffuseTerm = scene.sceneMetaData.globalData.kd * material.cDiffuse * diffuseClamped;
        if (m_config.enableTextureMap) {
            diffuseTerm =  (material.blend * glm::vec4(textureColor, 1) + (1 - material.blend) * scene.sceneMetaData.globalData.kd * material.cDiffuse) * diffuseClamped;
        }

        // Specular term
        glm::vec3 r = 2 * glm::dot(Li, normal) * normal - Li;
        float specularDot = glm::dot(r, directionToCamera);
        float specularDotClamped = glm::clamp(specularDot, 0.0f, 1.0f);
        glm::vec4 specularTerm = scene.sceneMetaData.globalData.ks * static_cast<float>(pow(specularDotClamped, material.shininess)) * material.cSpecular;

        // Calculate shadow
        bool isShadowIntersect = false;
        float softShadowFactor = 1.0f;  // default to no shadow
        bool enableSoftShadow = true; // TODO: Debug the flag handler!
        if (m_config.enableShadow) {
            intersectPos = cameraPos + (t - 0.01f) * d; // Avoid self-intersection

            glm::vec4 directionToLight;

            if (light.type == LightType::LIGHT_DIRECTIONAL) {
                directionToLight = -light.dir;
            } else {
                directionToLight = light.pos - intersectPos;
            }
            directionToLight = glm::normalize(directionToLight);

//            std::cout << m_config.enableSoftShadows << std::endl;
            // If soft shadow is enabled, trace ray to light on a finite area instead of a point.
            // Note: This is only available for point light and spot light. Direcectional light has infinite area.
            if (enableSoftShadow && light.type != LightType::LIGHT_DIRECTIONAL) {
                int numSamples = 20; // Adjust as needed
                int unobstructedCount = 0;

                for (int sample = 0; sample < numSamples; ++sample) {
                    float halfWidth = 0.25; // Adjust as needed
                    float halfHeight = 0.25; // Adjust as needed

                    glm::vec4 randomOffset((static_cast<float>(rand()) / RAND_MAX - 0.5) * 2 * halfWidth,
                                           (static_cast<float>(rand()) / RAND_MAX - 0.5) * 2 * halfHeight, 0, 0);
                    glm::vec4 lightSamplePoint = light.pos + randomOffset;
                    directionToLight = glm::normalize(lightSamplePoint - intersectPos);

                    float shadowT = 1000; // used to track the nearest intersection
                    glm::vec3 shadowIntersectNormal{-1, -1, -1};
                    RenderShapeData shadowIntersectShape; // The intersected shape that used to calculate lighting

                    bool currentShadowIntersect;
                    if (m_config.enableAcceleration) {
                        std::vector<RenderShapeData> potentialShadowIntersectShapes = m_bvh->potentialIntersections(intersectPos, directionToLight); // BVH version
                        currentShadowIntersect = calculateIntersection(potentialShadowIntersectShapes, intersectPos, directionToLight, shadowT, shadowIntersectNormal, shadowIntersectShape);
                    } else {
                        std::vector<RenderShapeData> potentialShadowIntersectShapes = scene.sceneMetaData.shapes;
                        currentShadowIntersect = calculateIntersection(potentialShadowIntersectShapes, intersectPos, directionToLight, shadowT, shadowIntersectNormal, shadowIntersectShape);
                    }

                    if (!currentShadowIntersect) {
                        unobstructedCount++;
                    }
                }

                softShadowFactor = static_cast<float>(unobstructedCount) / numSamples;

                illumination.x += softShadowFactor * att * color.x * (diffuseTerm.x + specularTerm.x) * (1-falloff);
                illumination.y += softShadowFactor * att * color.y * (diffuseTerm.y + specularTerm.y) * (1-falloff);
                illumination.z += softShadowFactor * att * color.z * (diffuseTerm.z + specularTerm.z) * (1-falloff);

            } else {
                float shadowT = 1000; // used to track the nearest intersection
                glm::vec3 shadowIntersectNormal{-1, -1, -1};
                RenderShapeData shadowIntersectShape; // The intersected shape that used to calculate lighting

                if (m_config.enableAcceleration) {
                    std::vector<RenderShapeData> potentialShadowIntersectShapes = m_bvh->potentialIntersections(intersectPos, directionToLight); // BVH version
                    isShadowIntersect = calculateIntersection(potentialShadowIntersectShapes, intersectPos, directionToLight, shadowT, shadowIntersectNormal, shadowIntersectShape);
                } else {
                    std::vector<RenderShapeData> potentialShadowIntersectShapes = scene.sceneMetaData.shapes;
                    isShadowIntersect = calculateIntersection(potentialShadowIntersectShapes, intersectPos, directionToLight, shadowT, shadowIntersectNormal, shadowIntersectShape);
                }

                illumination.x += !isShadowIntersect * att * color.x * (diffuseTerm.x + specularTerm.x) * (1-falloff);
                illumination.y += !isShadowIntersect * att * color.y * (diffuseTerm.y + specularTerm.y) * (1-falloff);
                illumination.z += !isShadowIntersect * att * color.z * (diffuseTerm.z + specularTerm.z) * (1-falloff);
            }
        }
    }
}

void RayTracer::mapNormalColor(glm::vec3 inColor, glm::vec3 &outColor) {
    outColor.x = (inColor.x + 1.0f) * 0.5f * 255.0f;
    outColor.y = (inColor.y + 1.0f) * 0.5f * 255.0f;
    outColor.z = (inColor.z + 1.0f) * 0.5f * 255.0f;
}

void RayTracer::mapIlluminationColor(glm::vec3 inColor, glm::vec3 &outColor) {
    outColor.x = glm::clamp(inColor.x, 0.0f, 1.0f) * 255.0f;
    outColor.y = glm::clamp(inColor.y, 0.0f, 1.0f) * 255.0f;
    outColor.z = glm::clamp(inColor.z, 0.0f, 1.0f) * 255.0f;
}
