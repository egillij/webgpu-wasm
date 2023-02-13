// Copyright 2023 Egill Ingi Jacobsen

#pragma once

#include <glm/mat4x4.hpp>

struct SceneUniforms {
    glm::mat4 viewProjection;
    glm::mat4 view;
    glm::vec4 cameraPosition;
};

struct ModelUniforms {
    glm::mat4 transform;
    glm::mat4 normalMatrix;
};