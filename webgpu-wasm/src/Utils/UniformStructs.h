#pragma once

#include <glm/mat4x4.hpp>

struct SceneUniforms {
    glm::mat4 viewProjection;
    glm::vec4 cameraPosition;
};

struct ModelUniforms {
    glm::mat4 transform;
    glm::mat4 normalMatrix;
};