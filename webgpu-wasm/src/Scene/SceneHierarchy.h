// Copyright 2023 Egill Ingi Jacobsen

#pragma once

#include "Scene/GameObject.h"

#include <glm/mat4x3.hpp>

#include <vector>


class SceneHierarchy {
public:
    SceneHierarchy();
    ~SceneHierarchy();

    struct Node {
        GameObject* gameObject = nullptr;
        glm::mat4 transform = glm::mat4(1.f);

        Node* child = nullptr;

        Node* next = nullptr;
    };

    Node* getRoot() { return m_Root; }

private:
    Node* m_Root;
};