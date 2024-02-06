#include <stdexcept>
#include "raytracescene.h"
#include "utils/sceneparser.h"

RayTraceScene::RayTraceScene(int width, int height, const RenderData &metaData) {
    sceneWidth = width;
    sceneHeight = height;
    sceneMetaData = metaData;

    // Setup the camera
    SceneCameraData cameraData = sceneMetaData.cameraData;
    sceneCamera.setup(sceneWidth, sceneHeight, cameraData.pos, cameraData.look, cameraData.up, cameraData.heightAngle, cameraData.aperture, cameraData.focalLength);
}

const int& RayTraceScene::width() const {
    return sceneWidth;
}

const int& RayTraceScene::height() const {
    return sceneHeight;
}

const SceneGlobalData& RayTraceScene::getGlobalData() const {
    return sceneMetaData.globalData;
}

const Camera& RayTraceScene::getCamera() const {   
    return sceneCamera;
}
