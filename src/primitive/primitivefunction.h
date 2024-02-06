#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <iostream>
#include <QObject>
#include <QImage>
#include "utils/rgba.h"

using TNormalTuple = std::tuple<float, glm::vec3>;

struct Triangle {
    glm::vec3 v0, v1, v2; // vertices
};

struct ImageData {
    std::vector<RGBA> data;
    int width;
    int height;
};

class PrimitiveFunction
{
public:
    PrimitiveFunction();

    TNormalTuple triangleIntersect(glm::vec4 p, glm::vec4 d, const Triangle& triangle);
    glm::vec3 triangleNormal(const Triangle& triangle);

    TNormalTuple cubeIntersect(glm::vec4 p, glm::vec4 d);
    TNormalTuple sphereIntersect(glm::vec4 p, glm::vec4 d);
    TNormalTuple cylinderIntersect(glm::vec4 p, glm::vec4 d);
    TNormalTuple coneIntersect(glm::vec4 p, glm::vec4 d);

    TNormalTuple sphereIntersectInside(glm::vec4 p, glm::vec4 d);
    TNormalTuple cubeIntersectFromInside(glm::vec4 p, glm::vec4 d);
    TNormalTuple cylinderIntersectInside(glm::vec4 p, glm::vec4 d);
    TNormalTuple coneIntersectInside(glm::vec4 p, glm::vec4 d);

    glm::vec3 sphereNormal(glm::vec4 p, glm::vec4 d, float t);
    glm::vec3 cubeNormal(glm::vec4 p, glm::vec4 d, float t);
    glm::vec3 cylinderRoundNormal(glm::vec4 p, glm::vec4 d, float t);
    glm::vec3 coneTopNormal(glm::vec4 p, glm::vec4 d, float t);

    glm::vec3 sphereTexture(bool isFilter, glm::vec4 p, glm::vec4 d, float t, float repeatU, float repeatV, QString imgPath);
    glm::vec3 cubeTexture(bool isFilter, glm::vec4 p, glm::vec4 d, float t, float repeatU, float repeatV, QString imgPath);
    glm::vec3 cylinderTexture(bool isFilter, glm::vec4 p, glm::vec4 d, float t, float repeatU, float repeatV, QString imgPath);
    glm::vec3 coneTexture(bool isFilter, glm::vec4 p, glm::vec4 d, float t, float repeatU, float repeatV, QString imgPath);

    std::vector<RGBA> loadImage(const QString &filePath, int &width, int &height);

    glm::vec3 bilinearFiltering(float u_prime, float v_prime, ImageData imgData);
    float bilinearInterpolate(float A, float B, float alpha);
    glm::vec3 biCubicFiltering(float u_prime, float v_prime, ImageData imgData);
    glm::vec3 cubicInterpolate(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float alpha);

    RGBA getPixelRepeated(std::vector<RGBA> &data, int width, int height, int x, int y);

    void sortTuples(std::vector<TNormalTuple>& tupleList);
    glm::vec3 getNormalFromSmallestT(const std::vector<TNormalTuple>& tupleList);
    TNormalTuple getFirstTuple(const std::vector<TNormalTuple>& tupleList);
    TNormalTuple getLastTuple(const std::vector<TNormalTuple>& intersectionTuples);
    bool checkInsideCube(float t, glm::vec4 p, glm::vec4 d);
    float calNearestIntersection(std::vector<float> intersections);
    float calDiscriminant(float a, float b, float c);
};
