#pragma once

#include <cstdint>
#include <glm/mat4x4.hpp>

struct Entity {
    uint32_t id;
};

struct MeshComponent {
    
};

struct TransformComponent {
    glm::mat4 transform;
};

// Very simple Entity Component System
class ECS final {
public:


private:
};