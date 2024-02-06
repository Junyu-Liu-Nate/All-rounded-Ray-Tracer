#include "primitivefunction.h"
//#include <QPainter>
//#include <QMessageBox>
//#include <QFileDialog>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include "utils/rgba.h"

PrimitiveFunction::PrimitiveFunction()
{

}

using TNormalTuple = std::tuple<float, glm::vec3>;

std::map<QString, ImageData> imageCache;

// Mesh Triangle
TNormalTuple PrimitiveFunction::triangleIntersect(glm::vec4 p, glm::vec4 d, const Triangle& triangle) {
    const float EPSILON = 0.0000001f;
    glm::vec3 e1, e2, h, s, q;
    float a, f, u, v;

    e1 = triangle.v1 - triangle.v0;
    e2 = triangle.v2 - triangle.v0;

    h = glm::cross(glm::vec3(d), e2);
    a = glm::dot(e1, h);

    if (a > -EPSILON && a < EPSILON) {
        return std::make_tuple(-1, glm::vec3(-1, -1, -1));
    }

    f = 1.0f / a;
    s = glm::vec3(p) - triangle.v0;
    u = f * glm::dot(s, h);

    if (u < 0.0f || u > 1.0f) {
        return std::make_tuple(-1, glm::vec3(-1, -1, -1));
    }

    q = glm::cross(s, e1);
    v = f * glm::dot(glm::vec3(d), q);

    if (v < 0.0f || u + v > 1.0f) {
        return std::make_tuple(-1, glm::vec3(-1, -1, -1));
    }

    float t = f * glm::dot(e2, q);

    if (t > EPSILON) {
        return std::make_tuple(t, triangleNormal(triangle));
    }

    return std::make_tuple(-1, glm::vec3(-1, -1, -1));
}


glm::vec3 PrimitiveFunction::triangleNormal(const Triangle& triangle) {
    glm::vec3 e1 = triangle.v1 - triangle.v0;
    glm::vec3 e2 = triangle.v2 - triangle.v0;

    glm::vec3 normal = glm::normalize(glm::cross(e1, e2));
    return normal;
}


// Sphere
TNormalTuple PrimitiveFunction::sphereIntersect(glm::vec4 p, glm::vec4 d) {
    // Implicit functions
    float a = d.x * d.x + d.y * d.y + d.z * d.z;
    float b = 2 * (p.x * d.x + p.y * d.y + p.z * d.z);
    float c = p.x * p.x + p.y * p.y + p.z * p.z - 0.25;
    float discriminant = calDiscriminant(a, b, c);

    std::vector<TNormalTuple> intersectionTuples;

    // Calculate intersections
    std::vector<float> intersections;
    if (discriminant == 0) {
        float t = -b / (2*a);
        intersectionTuples.push_back(std::make_tuple(t, sphereNormal(p, d, t)));
    }
    if (discriminant > 0) {
        float t1 = (-b + sqrt(discriminant)) / (2*a);
        float t2 = (-b - sqrt(discriminant)) / (2*a);
        intersectionTuples.push_back(std::make_tuple(t1, sphereNormal(p, d, t1)));
        intersectionTuples.push_back(std::make_tuple(t2, sphereNormal(p, d, t2)));
    }

    sortTuples(intersectionTuples);

    return getFirstTuple(intersectionTuples);
}

TNormalTuple PrimitiveFunction::sphereIntersectInside(glm::vec4 p, glm::vec4 d) {
    // Implicit functions
    float a = d.x * d.x + d.y * d.y + d.z * d.z;
    float b = 2 * (p.x * d.x + p.y * d.y + p.z * d.z);
    float c = p.x * p.x + p.y * p.y + p.z * p.z - 0.25;
    float discriminant = calDiscriminant(a, b, c);

    std::vector<TNormalTuple> intersectionTuples;

    if (discriminant < 0) {
        // This means there's something wrong. When inside the sphere, the discriminant should never be negative.
        return {}; // Handle this case accordingly
    }

    // Calculate intersections
    float t1 = (-b + sqrt(discriminant)) / (2*a);
    float t2 = (-b - sqrt(discriminant)) / (2*a);

    // Since the point is inside the sphere, t2 is the point where the ray exits the sphere
    intersectionTuples.push_back(std::make_tuple(t2, sphereNormal(p, d, t2)));

    sortTuples(intersectionTuples);

    return getFirstTuple(intersectionTuples);
}

glm::vec3 PrimitiveFunction::sphereNormal(glm::vec4 p, glm::vec4 d, float t) {
    float x = p.x + t * d.x;
    float y = p.y + t * d.y;
    float z = p.z + t * d.z;

    // Gradient
    glm::vec3 normal(2*x, 2*y, 2*z);

    return normal;
}

glm::vec3 PrimitiveFunction::sphereTexture(bool isFilter, glm::vec4 p, glm::vec4 d, float t, float repeatU, float repeatV, QString imgPath) {
    float x = p.x + t * d.x;
    float y = p.y + t * d.y;
    float z = p.z + t * d.z;

    float u;
    float theta = atan2(z, x);
    if (theta < 0) {
        u = -theta / (2 * M_PI);
    } else {
        u = 1 - theta / (2 * M_PI);
    }

    float v = asin(y / 0.5) / M_PI + 0.5;

    // Check cache for the image
    auto iter = imageCache.find(imgPath);
    ImageData imgData;

    if (iter == imageCache.end()) {
        int width, height;
        auto image = loadImage(imgPath, width, height);

        imgData.data = image;
        imgData.width = width;
        imgData.height = height;

        imageCache[imgPath] = imgData;
    } else {
        imgData = iter->second;
    }

    int segment_u = static_cast<int>(u * repeatU);
    float u_prime = u * repeatU - segment_u;

    int segment_v = static_cast<int>(v * repeatV);
    float v_prime = v * repeatV - segment_v;

    if (!isFilter) {
        int c = static_cast<int>(u_prime * imgData.width);
        int r = imgData.height - 1 - static_cast<int>(v_prime * imgData.height);

        RGBA pixelColor = imgData.data.at(c + r * imgData.width);

        return glm::vec3(pixelColor.r / 255.0f, pixelColor.g / 255.0f, pixelColor.b / 255.0f);
    }
    else {
//        return bilinearFiltering(u_prime, v_prime, imgData);
        return biCubicFiltering(u_prime, v_prime, imgData);
    }
}


// Cube
TNormalTuple PrimitiveFunction::cubeIntersect(glm::vec4 p, glm::vec4 d) {
    std::vector<TNormalTuple> intersectionTuples;

    float t1 = (0.5 - p.x) / d.x;
    float y = p.y + t1 * d.y;
    float z = p.z + t1 * d.z;
    if (y >= -0.5 && y <= 0.5 && z >= -0.5 && z <= 0.5) {
        intersectionTuples.push_back(std::make_tuple(t1, glm::vec3 (1, 0, 0)));
    }

    float t2 = (-0.5 - p.x) / d.x;
    y = p.y + t2 * d.y;
    z = p.z + t2 * d.z;
    if (y >= -0.5 && y <= 0.5 && z >= -0.5 && z <= 0.5) {
        intersectionTuples.push_back(std::make_tuple(t2, glm::vec3 (-1, 0, 0)));
    }

    float t3 = (0.5 - p.y) / d.y;
    float x = p.x + t3 * d.x;
    z = p.z + t3 * d.z;
    if (x >= -0.5 && x <= 0.5 && z >= -0.5 && z <= 0.5) {
        intersectionTuples.push_back(std::make_tuple(t3, glm::vec3 (0, 1, 0)));
    }

    float t4 = (-0.5 - p.y) / d.y;
    x = p.x + t4 * d.x;
    z = p.z + t4 * d.z;
    if (x >= -0.5 && x <= 0.5 && z >= -0.5 && z <= 0.5) {
        intersectionTuples.push_back(std::make_tuple(t4, glm::vec3 (0, -1, 0)));
    }

    float t5 = (0.5 - p.z) / d.z;
    x = p.x + t5 * d.x;
    y = p.y + t5 * d.y;
    if (x >= -0.5 && x <= 0.5 && y >= -0.5 && y <= 0.5) {
        intersectionTuples.push_back(std::make_tuple(t5, glm::vec3 (0, 0, 1)));
    }

    float t6 = (-0.5 - p.z) / d.z;
    x = p.x + t6 * d.x;
    y = p.y + t6 * d.y;
    if (x >= -0.5 && x <= 0.5 && y >= -0.5 && y <= 0.5) {
        intersectionTuples.push_back(std::make_tuple(t6, glm::vec3 (0, 0, -1)));
    }

    sortTuples(intersectionTuples);

    return getFirstTuple(intersectionTuples);
}

TNormalTuple PrimitiveFunction::cubeIntersectFromInside(glm::vec4 p, glm::vec4 d) {
    std::vector<TNormalTuple> intersectionTuples;

    // Check each face for intersections:
    for (int i = 0; i < 3; ++i) {
        float positive_face = (0.5 - p[i]) / d[i];
        float negative_face = (-0.5 - p[i]) / d[i];

        // Swap if necessary to make sure positive_face has the positive t value:
        if (positive_face < 0) {
            std::swap(positive_face, negative_face);
        }

        glm::vec3 pos_normal(0);
        glm::vec3 neg_normal(0);
        pos_normal[i] = 1;
        neg_normal[i] = -1;

        // Now, positive_face is your exit point.
        if (checkInsideCube(positive_face, p, d)) {
            intersectionTuples.push_back(std::make_tuple(positive_face, pos_normal));
        }
    }

    sortTuples(intersectionTuples);

    return getFirstTuple(intersectionTuples);
}

bool PrimitiveFunction::checkInsideCube(float t, glm::vec4 p, glm::vec4 d) {
    float x = p.x + t * d.x;
    float y = p.y + t * d.y;
    float z = p.z + t * d.z;

    if (x >= -0.5 && x <= 0.5 && y >= -0.5 && y <= 0.5 && z >= -0.5 && z <= 0.5) {
        return true;
    }
    else {
        return false;
    }
}

glm::vec3 PrimitiveFunction::cubeTexture(bool isFilter, glm::vec4 p, glm::vec4 d, float t, float repeatU, float repeatV, QString imgPath) {
    const float epsilon = 1e-5f;  // Epsilon for floating point comparisons

    float x = p.x + t * d.x;
    float y = p.y + t * d.y;
    float z = p.z + t * d.z;

    float u, v;
    glm::vec3 normal;

    // Identify the face that's hit and compute u, v coordinates
    if (std::abs(x - 0.5) < epsilon) {
        u = (-z + 0.5);
        v = (y + 0.5);
    }
    else if (std::abs(x + 0.5) < epsilon) {
        u = (z + 0.5);
        v = (y + 0.5);
    }
    else if (std::abs(y - 0.5) < epsilon) {
        u = (x + 0.5);
        v = (-z + 0.5);
    }
    else if (std::abs(y + 0.5) < epsilon) {
        u = (x + 0.5);
        v = (z + 0.5);
    }
    else if (std::abs(z - 0.5) < epsilon) {
        u = (x + 0.5);
        v = (y + 0.5);
    }
    else {  // z + 0.5
        u = (-x + 0.5);
        v = (y + 0.5);
    }

    // Check cache for the image
    auto iter = imageCache.find(imgPath);
    ImageData imgData;

    if (iter == imageCache.end()) {
        int width, height;
        auto image = loadImage(imgPath, width, height);

        imgData.data = image;
        imgData.width = width;
        imgData.height = height;

        imageCache[imgPath] = imgData;
    } else {
        imgData = iter->second;
    }

    int segment_u = static_cast<int>(u * repeatU);
    float u_prime = u * repeatU - segment_u;

    int segment_v = static_cast<int>(v * repeatV);
    float v_prime = v * repeatV - segment_v;

    if (!isFilter) {
        int c = static_cast<int>(u_prime * imgData.width);
        int r = imgData.height - 1 - static_cast<int>(v_prime * imgData.height);

        RGBA pixelColor = imgData.data.at(c + r * imgData.width);

        return glm::vec3(pixelColor.r / 255.0f, pixelColor.g / 255.0f, pixelColor.b / 255.0f);
    }
    else {
        return bilinearFiltering(u_prime, v_prime, imgData);
//        return biCubicFiltering(u_prime, v_prime, imgData);
    }
}


//Cylinder
TNormalTuple PrimitiveFunction::cylinderIntersect(glm::vec4 p, glm::vec4 d) {
    // Implicit functions for infinite cylinder
    float a = d.x * d.x + d.z * d.z;
    float b = 2 * (p.x * d.x + p.z * d.z);
    float c = p.x * p.x + p.z * p.z - 0.25;
    float discriminant = calDiscriminant(a, b, c);

    std::vector<TNormalTuple> intersectionTuples;

    // Calculate intersections with infinite cylinder
    if (discriminant == 0) {
        float t = -b / (2*a);
        float y = p.y + t * d.y;
        if (y >= -0.5 && y <= 0.5) {
            intersectionTuples.push_back(std::make_tuple(t, cylinderRoundNormal(p, d, t)));
        }
    }
    if (discriminant > 0) {
        float t1 = (-b + sqrt(discriminant)) / (2*a);
        float t2 = (-b - sqrt(discriminant)) / (2*a);
        float y1 = p.y + t1 * d.y;
        if (y1 >= -0.5 && y1 <= 0.5) {
            intersectionTuples.push_back(std::make_tuple(t1, cylinderRoundNormal(p, d, t1)));
        }
        float y2 = p.y + t2 * d.y;
        if (y2 >= -0.5 && y2 <= 0.5) {
            intersectionTuples.push_back(std::make_tuple(t2, cylinderRoundNormal(p, d, t2)));
        }
    }

    float t3 = (0.5 - p.y) / d.y;
    float x3 = p.x + t3 * d.x;
    float z3 = p.z + t3 * d.z;
    if (x3 * x3 + z3 * z3 <= 0.25) {
        intersectionTuples.push_back(std::make_tuple(t3, glm::vec3 (0, 1, 0)));
    }

    float t4 = (-0.5 - p.y) / d.y;
    float x4 = p.x + t4 * d.x;
    float z4 = p.z + t4 * d.z;
    if (x4 * x4 + z4 * z4 <= 0.25) {
        intersectionTuples.push_back(std::make_tuple(t4, glm::vec3 (0, -1, 0)));
    }

    sortTuples(intersectionTuples);

    return getFirstTuple(intersectionTuples);
}

TNormalTuple PrimitiveFunction::cylinderIntersectInside(glm::vec4 p, glm::vec4 d) {
    float a = d.x * d.x + d.z * d.z;
    float b = 2 * p.x * d.x + 2 * p.z * d.z;
    float c = p.x * p.x + p.z * p.z - 0.25;

    std::vector<TNormalTuple> intersectionTuples;

    float discriminant = calDiscriminant(a, b, c);
    if (discriminant >= 0) {
        float t1 = (-b + sqrt(discriminant)) / (2*a);
        float y1 = p.y + t1 * d.y;
        if (y1 >= -0.5 && y1 <= 0.5) {
            intersectionTuples.push_back(std::make_tuple(t1, glm::vec3(0, 1, 0)));
        }

        float t2 = (-b - sqrt(discriminant)) / (2*a);
        float y2 = p.y + t2 * d.y;
        if (y2 >= -0.5 && y2 <= 0.5) {
            intersectionTuples.push_back(std::make_tuple(t2, glm::vec3(0, 1, 0)));
        }
    }

    float t4 = (-0.5 - p.y) / d.y;
    float x4 = p.x + t4 * d.x;
    float z4 = p.z + t4 * d.z;
    if (x4 * x4 + z4 * z4 <= 0.25) {
        intersectionTuples.push_back(std::make_tuple(t4, glm::vec3 (0, -1, 0)));
    }

    float t3 = (0.5 - p.y) / d.y;
    float x3 = p.x + t3 * d.x;
    float z3 = p.z + t3 * d.z;
    if (x3 * x3 + z3 * z3 <= 0.25) {
        intersectionTuples.push_back(std::make_tuple(t3, glm::vec3 (0, 1, 0)));
    }

    sortTuples(intersectionTuples);

    return getLastTuple(intersectionTuples);  // farthest intersection.
}


glm::vec3 PrimitiveFunction::cylinderRoundNormal(glm::vec4 p, glm::vec4 d, float t) {
    float x = p.x + t * d.x;
    float y = p.y + t * d.y;
    float z = p.z + t * d.z;

    // Gradient
    glm::vec3 normal(2*x, 0*y, 2*z);

    return normal;
}

glm::vec3 PrimitiveFunction::cylinderTexture(bool isFilter, glm::vec4 p, glm::vec4 d, float t, float repeatU, float repeatV, QString imgPath) {
    float x = p.x + t * d.x;
    float y = p.y + t * d.y;
    float z = p.z + t * d.z;

    float u, v;

    glm::vec3 normal = cylinderRoundNormal(p, d, t);

    float epsilon = 1e-5;  // Added epsilon for floating-point precision issues

    if (std::abs(y + 0.5) < epsilon || std::abs(y - 0.5) < epsilon) {  // Adjusted for precision
        if (std::abs(y + 0.5) < epsilon) {
            u = x + 0.5;
            v = z + 0.5;
        } else {
            u = x + 0.5;
            v = -z + 0.5;
        }
    } else {  // Side of the cylinder
        float theta = atan2(z, x);
        if (theta < 0) {
            u = -theta / (2 * M_PI);
        } else {
            u = 1 - theta / (2 * M_PI);
        }

        v = y + 0.5;  // Maps [-1,1] to [0,1]
    }

    // Check cache for the image
    auto iter = imageCache.find(imgPath);
    ImageData imgData;

    if (iter == imageCache.end()) {
        int width, height;
        auto image = loadImage(imgPath, width, height);

        imgData.data = image;
        imgData.width = width;
        imgData.height = height;

        imageCache[imgPath] = imgData;
    } else {
        imgData = iter->second;
    }

    int segment_u = static_cast<int>(u * repeatU);
    float u_prime = u * repeatU - segment_u;

    int segment_v = static_cast<int>(v * repeatV);
    float v_prime = v * repeatV - segment_v;

    if (!isFilter) {
        int c = static_cast<int>(u_prime * imgData.width);
        int r = imgData.height - 1 - static_cast<int>(v_prime * imgData.height);

        RGBA pixelColor = imgData.data.at(c + r * imgData.width);

        return glm::vec3(pixelColor.r / 255.0f, pixelColor.g / 255.0f, pixelColor.b / 255.0f);
    }
    else {
        return bilinearFiltering(u_prime, v_prime, imgData);
//        return biCubicFiltering(u_prime, v_prime, imgData);
    }
}

// Cone
TNormalTuple PrimitiveFunction::coneIntersect(glm::vec4 p, glm::vec4 d) {
    // Implicit functions
    float a = d.x * d.x + d.z * d.z - 0.25 * d.y * d.y;
    float b = 2 * p.x * d.x + 2 * p.z * d.z - 0.5 * p.y * d.y + 0.25 * d.y;
    float c = p.x * p.x + p.z * p.z - 0.25 * p.y * p.y + 0.25 * p.y - 0.0625;
    float discriminant = calDiscriminant(a, b, c);

    std::vector<TNormalTuple> intersectionTuples;

    // Calculate intersections with infinite cylinder
    if (discriminant == 0) {
        float t = -b / (2*a);
        float y = p.y + t * d.y;
        if (y >= -0.5 && y <= 0.5) {
            intersectionTuples.push_back(std::make_tuple(t, coneTopNormal(p, d, t)));
        }
    }
    if (discriminant > 0) {
        float t1 = (-b + sqrt(discriminant)) / (2*a);
        float t2 = (-b - sqrt(discriminant)) / (2*a);
        float y1 = p.y + t1 * d.y;
        if (y1 >= -0.5 && y1 <= 0.5) {
            intersectionTuples.push_back(std::make_tuple(t1, coneTopNormal(p, d, t1)));
        }
        float y2 = p.y + t2 * d.y;
        if (y2 >= -0.5 && y2 <= 0.5) {
            intersectionTuples.push_back(std::make_tuple(t2, coneTopNormal(p, d, t2)));
        }
    }

    float t3 = (-0.5 - p.y) / d.y;
    float x3 = p.x + t3 * d.x;
    float z3 = p.z + t3 * d.z;
    if (x3 * x3 + z3 * z3 <= 0.25) {
        intersectionTuples.push_back(std::make_tuple(t3, glm::vec3 (0, -1, 0)));
    }

    sortTuples(intersectionTuples);

    return getFirstTuple(intersectionTuples);
}

TNormalTuple PrimitiveFunction::coneIntersectInside(glm::vec4 p, glm::vec4 d) {
    float a = d.x * d.x + d.z * d.z - 0.25 * d.y * d.y;
    float b = 2 * p.x * d.x + 2 * p.z * d.z - 0.5 * p.y * d.y + 0.25 * d.y;
    float c = p.x * p.x + p.z * p.z - 0.25 * p.y * p.y + 0.25 * p.y - 0.0625;

    std::vector<TNormalTuple> intersectionTuples;

    float discriminant = calDiscriminant(a, b, c);
    if (discriminant == 0) {
        float t = -b / (2*a);
        float y = p.y + t * d.y;
        if (y >= -0.5 && y <= 0.5) {
            intersectionTuples.push_back(std::make_tuple(t, coneTopNormal(p, d, t)));
        }
    }
    if (discriminant > 0) {
        float t1 = (-b + sqrt(discriminant)) / (2*a);
        float y1 = p.y + t1 * d.y;
        if (y1 >= -0.5 && y1 <= 0.5) {
            intersectionTuples.push_back(std::make_tuple(t1, coneTopNormal(p, d, t1)));
        }
        float t2 = (-b - sqrt(discriminant)) / (2*a);
        float y2 = p.y + t2 * d.y;
        if (y2 >= -0.5 && y2 <= 0.5) {
            intersectionTuples.push_back(std::make_tuple(t2, coneTopNormal(p, d, t2)));
        }
    }

    float t3 = (-0.5 - p.y) / d.y;
    float x3 = p.x + t3 * d.x;
    float z3 = p.z + t3 * d.z;
    if (x3 * x3 + z3 * z3 <= 0.25) {
        intersectionTuples.push_back(std::make_tuple(t3, glm::vec3 (0, -1, 0)));
    }

    sortTuples(intersectionTuples);

    return getLastTuple(intersectionTuples);  // farthest intersection.
}


glm::vec3 PrimitiveFunction::coneTopNormal(glm::vec4 p, glm::vec4 d, float t) {
    float x = p.x + t * d.x;
    float y = p.y + t * d.y;
    float z = p.z + t * d.z;

    // Gradient
    glm::vec3 normal(x/sqrt(x*x + z*z), 0.5, z/sqrt(x*x + z*z));

    return normal;
}

glm::vec3 PrimitiveFunction::coneTexture(bool isFilter, glm::vec4 p, glm::vec4 d, float t, float repeatU, float repeatV, QString imgPath) {
    // Calculate intersection point on the cone
    float x = p.x + t * d.x;
    float y = p.y + t * d.y;
    float z = p.z + t * d.z;

    float epsilon = 1e-5;  // Added epsilon for floating-point precision issues

    float u = 0;
    float v = 0;
    if (std::abs(y + 0.5) < epsilon) {
        u = x + 0.5;
        v = z + 0.5;
    }
    else {
        // For U (azimuthal angle):
        float theta = atan2(z, x);
        if (theta < 0) {
            u = -theta / (2 * M_PI);
        } else {
            u = 1 - theta / (2 * M_PI);
        }

        // For V (height along the cone):
        v = y + 0.5;
    }

    // Check cache for the image
    auto iter = imageCache.find(imgPath);
    ImageData imgData;

    if (iter == imageCache.end()) {
        int width, height;
        auto image = loadImage(imgPath, width, height);

        imgData.data = image;
        imgData.width = width;
        imgData.height = height;

        imageCache[imgPath] = imgData;
    } else {
        imgData = iter->second;
    }

    int segment_u = static_cast<int>(u * repeatU);
    float u_prime = u * repeatU - segment_u;

    int segment_v = static_cast<int>(v * repeatV);
    float v_prime = v * repeatV - segment_v;

    if (!isFilter) {
        int c = static_cast<int>(u_prime * imgData.width);
        int r = imgData.height - 1 - static_cast<int>(v_prime * imgData.height);

        RGBA pixelColor = imgData.data.at(c + r * imgData.width);

        return glm::vec3(pixelColor.r / 255.0f, pixelColor.g / 255.0f, pixelColor.b / 255.0f);
    }
    else {
//        return bilinearFiltering(u_prime, v_prime, imgData);
        return biCubicFiltering(u_prime, v_prime, imgData);
    }
}


/******************************* I/O functions *******************************/
std::vector<RGBA> PrimitiveFunction::loadImage(const QString &filePath, int &width, int &height) {
    QImage myImage;
    if (!myImage.load(filePath)) {
        std::cerr << "Failed to load in image" << std::endl;
        return {};
    }

    myImage = myImage.convertToFormat(QImage::Format_RGBX8888);
    QByteArray arr = QByteArray::fromRawData((const char*) myImage.bits(), myImage.sizeInBytes());

    width = myImage.width();
    height = myImage.height();

    std::vector<RGBA> image;
    image.reserve(width * height);
    for (int i = 0; i < arr.size() / 4; i++){
        image.push_back(RGBA{
            (std::uint8_t) arr[4*i],
            (std::uint8_t) arr[4*i+1],
            (std::uint8_t) arr[4*i+2],
            (std::uint8_t) arr[4*i+3]
        });
    }

    return image;
}

/******************************* Helper functions *******************************/
// Bilinear filtering
glm::vec3 PrimitiveFunction::bilinearFiltering(float u_prime, float v_prime, ImageData imgData) {
    float x = u_prime * imgData.width;
    float y = imgData.height - 1 - v_prime * imgData.height;

    // get grid indices (as ints)
    int xFloor = std::floor(x);
    int yFloor = std::floor(y);

    RGBA texelTL = getPixelRepeated(imgData.data, imgData.width, imgData.height, xFloor, yFloor);
    glm::vec3 colorsTL = glm::vec3(texelTL.r / 255.0f, texelTL.g / 255.0f, texelTL.b / 255.0f);
    RGBA texelTR = getPixelRepeated(imgData.data, imgData.width, imgData.height, xFloor + 1, yFloor);
    glm::vec3 colorsTR = glm::vec3(texelTR.r / 255.0f, texelTR.g / 255.0f, texelTR.b / 255.0f);
    RGBA texelBL = getPixelRepeated(imgData.data, imgData.width, imgData.height, xFloor, yFloor + 1);
    glm::vec3 colorsBL = glm::vec3(texelBL.r / 255.0f, texelBL.g / 255.0f, texelBL.b / 255.0f);
    RGBA texelBR = getPixelRepeated(imgData.data, imgData.width, imgData.height, xFloor + 1, yFloor + 1);
    glm::vec3 colorsBR = glm::vec3(texelBR.r / 255.0f, texelBR.g / 255.0f, texelBR.b / 255.0f);

    glm::vec3 interpolatedColors(0.0f, 0.0f, 0.0f);
    interpolatedColors.x = bilinearInterpolate(bilinearInterpolate(colorsTL.x, colorsTR.x, x-xFloor), bilinearInterpolate(colorsBL.x, colorsBR.x, x-xFloor), y-yFloor);
    interpolatedColors.y = bilinearInterpolate(bilinearInterpolate(colorsTL.y, colorsTR.y, x-xFloor), bilinearInterpolate(colorsBL.y, colorsBR.y, x-xFloor), y-yFloor);
    interpolatedColors.z = bilinearInterpolate(bilinearInterpolate(colorsTL.z, colorsTR.z, x-xFloor), bilinearInterpolate(colorsBL.z, colorsBR.z, x-xFloor), y-yFloor);

    interpolatedColors.x = glm::clamp(interpolatedColors.x, 0.0f, 1.0f);
    interpolatedColors.y = glm::clamp(interpolatedColors.y, 0.0f, 1.0f);
    interpolatedColors.z = glm::clamp(interpolatedColors.z, 0.0f, 1.0f);

    return interpolatedColors;
}

float PrimitiveFunction::bilinearInterpolate(float A, float B, float alpha) {
    float ease = 3 * std::pow(alpha, 2) - 2 * std::pow(alpha, 3);
    return A + ease * (B - A);
}

// Bicubic filtering
glm::vec3 PrimitiveFunction::biCubicFiltering(float u_prime, float v_prime, ImageData imgData) {
    float x = u_prime * imgData.width;
    float y = imgData.height - 1 - v_prime * imgData.height;

    // get grid indices (as ints)
    int xFloor = std::floor(x);
    int yFloor = std::floor(y);

    glm::vec3 colors[4][4];

    // Fetch the 16 surrounding texels
    for (int i = -1; i <= 2; ++i) {
        for (int j = -1; j <= 2; ++j) {
            RGBA texel = getPixelRepeated(imgData.data, imgData.width, imgData.height, xFloor + i, yFloor + j);
            colors[i + 1][j + 1] = glm::vec3(texel.r / 255.0f, texel.g / 255.0f, texel.b / 255.0f);
        }
    }

    // Interpolate along x for each y
    glm::vec3 values[4];
    for (int i = 0; i < 4; ++i) {
        values[i] = cubicInterpolate(colors[0][i], colors[1][i], colors[2][i], colors[3][i], x - xFloor);
    }

    // Interpolate along y
    glm::vec3 result = cubicInterpolate(values[0], values[1], values[2], values[3], y - yFloor);

    result.x = glm::clamp(result.x, 0.0f, 1.0f);
    result.y = glm::clamp(result.y, 0.0f, 1.0f);
    result.z = glm::clamp(result.z, 0.0f, 1.0f);

    return result;
}

glm::vec3 PrimitiveFunction::cubicInterpolate(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float alpha) {
    glm::vec3 a = 3.0f * p1 - 3.0f * p2 + p3 - p0;
    glm::vec3 b = 2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3;
    glm::vec3 c = p2 - p0;
    glm::vec3 d = 2.0f * p1;

    glm::vec3 result = ((a * alpha + b) * alpha + c) * alpha + d;
    return result * 0.5f; // The factor of 0.5 corrects the multiplication and ensures values are in the expected range
}


// Repeats the pixel on the edge of the image such that A,B,C,D looks like ...A,A,A,B,C,D,D,D...
RGBA PrimitiveFunction::getPixelRepeated(std::vector<RGBA> &data, int width, int height, int x, int y) {
    int newX = (x < 0) ? 0 : std::min(x, width  - 1);
    int newY = (y < 0) ? 0 : std::min(y, height - 1);
    return data[width * newY + newX];
}

// Function to sort the tuples by the float values
void PrimitiveFunction::sortTuples(std::vector<TNormalTuple>& tupleList) {
    std::sort(tupleList.begin(), tupleList.end(),
              [](const TNormalTuple &a, const TNormalTuple &b) {
                  return std::get<0>(a) < std::get<0>(b);
              });
}

// Function to get the normal from the tuple with the smallest t
glm::vec3 PrimitiveFunction::getNormalFromSmallestT(const std::vector<TNormalTuple>& tupleList) {
    if(!tupleList.empty()) {
        return std::get<1>(tupleList.front());
    }
    // Return a default value if the list is empty
    return glm::vec3(-1);
}

TNormalTuple PrimitiveFunction::getFirstTuple(const std::vector<TNormalTuple>& tupleList) {
    if(!tupleList.empty()) {
        return tupleList.front();
    }
    // Return a default value if the list is empty
    return std::make_tuple(-1, glm::vec3(-1));
}

TNormalTuple PrimitiveFunction::getLastTuple(const std::vector<TNormalTuple>& intersectionTuples) {
    if (intersectionTuples.empty()) {
        return std::make_tuple(INFINITY, glm::vec3(0, 0, 0));
    }
    return intersectionTuples.back();
}


float PrimitiveFunction::calNearestIntersection(std::vector<float> intersections) {
    // Find out the smallest non-negative value
    float smallestNonNegative = -1;
    for (float value : intersections) {
        if (value >= 0) {
            if (smallestNonNegative == -1 || value < smallestNonNegative) {
                smallestNonNegative = value;
            }
        }
    }

    return smallestNonNegative;
}

float PrimitiveFunction::calDiscriminant(float a, float b, float c) {
    return b * b - 4 * a * c;
}


