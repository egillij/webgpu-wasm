#pragma once

#include <glm/mat4x4.hpp>

struct SceneUniforms {
    glm::mat4 viewProjection;
};

struct ModelUniforms {
    glm::mat4 transform;
};