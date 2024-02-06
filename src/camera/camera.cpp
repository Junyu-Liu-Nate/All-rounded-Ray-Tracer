#include <stdexcept>
#include "camera.h"
#include <iostream>

void Camera::setup(int width, int height, glm::vec4 pos, glm::vec4 look, glm::vec4 up, float heightAngle, float aperture, float focalLength) {
    cameraPos = pos;
    cameraPos.w = 1;
    cameraLook = look;
    cameraUp = up;

    calculateViewMatrix();

    aspectRatio = (width * 1.0) / height;

    cameraHeightAngle = heightAngle;
    cameraAperture = aperture;
    cameraFocalLength = focalLength;
}

void Camera::calculateViewMatrix() {
    // Note: convert the vec4 to vec3 for cross product operation
    glm::vec3 w = -cameraLook.xyz();
    w = glm::normalize(w);
    glm::vec3 v = cameraUp.xyz() - glm::dot(cameraUp.xyz(), w) * w;
    v = glm::normalize(v);
    glm::vec3 u = glm::cross(v, w);

    // Rotation matrix
    glm::vec4 wExpand = glm::vec4(w, 0);
    glm::vec4 vExpand = glm::vec4(v, 0);
    glm::vec4 uExpand = glm::vec4(u, 0);

    glm::mat4 matRotateT = glm::mat4(uExpand, vExpand, wExpand, glm::vec4(0, 0, 0, 1));
    glm::mat4 matRotate = glm::transpose(matRotateT);

    // Translation matrix
    glm::mat4 matTranslate = glm::mat4(glm::vec4(1, 0, 0, 0), glm::vec4(0, 1, 0, 0), glm::vec4(0, 0, 1, 0), glm::vec4(-cameraPos.xyz(), 1));

    viewMatrix = matRotate * matTranslate;

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            std::cout << viewMatrix[i][j] << ' ';
        }
        std::cout << std::endl;
    }
}

glm::mat4 Camera::getViewMatrix() const {
    return viewMatrix;
}

glm::mat4 Camera::getViewMatrixInverse() const {
    return glm::inverse(viewMatrix);
}

float Camera::getAspectRatio() const {
    return aspectRatio;
}

float Camera::getHeightAngle() const {
    return cameraHeightAngle;
}

float Camera::getFocalLength() const {
    return cameraFocalLength;
}

float Camera::getAperture() const {
    return cameraAperture;
}
