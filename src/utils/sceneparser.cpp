#include "sceneparser.h"
#include "scenefilereader.h"
#include <glm/gtx/transform.hpp>

#include <chrono>
#include <iostream>

bool SceneParser::parse(std::string filepath, RenderData &renderData) {
    ScenefileReader fileReader = ScenefileReader(filepath);
    bool success = fileReader.readJSON();
    if (!success) {
        return false;
    }

    // Use Lab 5 code here
    renderData.globalData = fileReader.getGlobalData();
    renderData.cameraData = fileReader.getCameraData();

    SceneNode* root = fileReader.getRootNode();
    renderData.shapes.clear();
    renderData.lights.clear();

    glm::mat4 ctm = glm::mat4(1.0f);
    dfsParse(root, renderData.shapes, renderData.lights, ctm);

    return true;
}

void SceneParser::dfsParse(SceneNode* node, std::vector<RenderShapeData>& shapes, std::vector<SceneLightData>& lights, glm::mat4 ctm) {
    if (node == nullptr) {
        return;
    }
    else {
        // Calculate ctm
        glm::mat4 translation = glm::mat4(1.0f);
        glm::mat4 rotation = glm::mat4(1.0f);
        glm::mat4 scaling = glm::mat4(1.0f);
        for (SceneTransformation* transformation : node->transformations) {
            switch (transformation->type) {
                case TransformationType::TRANSFORMATION_TRANSLATE:
                    translation = glm::translate(translation, transformation->translate);
                    break;

                case TransformationType::TRANSFORMATION_ROTATE:
                    rotation = glm::rotate(rotation, transformation->angle, transformation->rotate);
                    break;

                case TransformationType::TRANSFORMATION_SCALE:
                    scaling = glm::scale(scaling, transformation->scale);
                    break;

                case TransformationType::TRANSFORMATION_MATRIX:
                    ctm = ctm * transformation->matrix;
                    break;
            }
        }
        ctm = ctm * translation * rotation * scaling;

        // Create a renderNode for every primitive and append to shapes
        for (ScenePrimitive* primitive : node->primitives) {
            RenderShapeData renderNode;

            renderNode.primitive.type = primitive->type;
            renderNode.primitive.material = primitive->material;
            renderNode.primitive.meshfile = primitive->meshfile;
            renderNode.ctm = ctm;

            shapes.push_back(renderNode);
        }

        // Create a SceneLight for every light and append to lights
        for (SceneLight* light : node->lights) {
            SceneLightData sceneLight;

            sceneLight.id = light->id;
            sceneLight.type = light->type;
            sceneLight.color = light->color;
            sceneLight.function = light->function;

            switch (light->type) {
            case LightType::LIGHT_POINT:
                // Update using ctm
                sceneLight.pos = ctm * glm::vec4(0,0,0,1);
                break;

            case LightType::LIGHT_DIRECTIONAL:
                // Update using ctm
                sceneLight.dir = ctm * light->dir;
                break;

            case LightType::LIGHT_SPOT:
                // Update using ctm
                sceneLight.pos = ctm * glm::vec4(0,0,0,1);
                sceneLight.dir = ctm * light->dir;
                sceneLight.penumbra = light->penumbra;
                sceneLight.angle = light->angle;
                break;
            }

            lights.push_back(sceneLight);
        }

        for (SceneNode* childNode : node->children) {
            dfsParse(childNode, shapes, lights, ctm);
        }
    }

}
