#pragma once

#include "Scene/Scene.h"

std::vector<ModelDescription> getThrantaParts(int startModelId)
{
    std::vector<ModelDescription> models;
    {
        ModelDescription model{};
        model.id = startModelId + 0;
        model.name = "Thranta-class_0002_veh_rep_destroyer_veh_rep_destroyer_metal02_d_0";
        model.filename = "Thranta-class_0002_veh_rep_destroyer_veh_rep_destroyer_metal02_d_0.geom";
        model.position = glm::vec3(0.f);
        model.scale = glm::vec3(1.f);
        model.rotation = glm::vec3(0.f);
        models.push_back(model);
    }
    {
        ModelDescription model{};
        model.id = startModelId + 1;
        model.name = "Thranta-class_0002_veh_rep_destroyer_veh_rep_destroyer_detail_d_0";
        model.filename = "Thranta-class_0002_veh_rep_destroyer_veh_rep_destroyer_detail_d_0.geom";
        model.position = glm::vec3(0.f);
        model.scale = glm::vec3(1.f);
        model.rotation = glm::vec3(0.f);
        models.push_back(model);
    }
    {
        ModelDescription model{};
        model.id = startModelId + 2;
        model.name = "Thranta-class_0002_veh_rep_destroyer_veh_rep_destroyer_metal01_d_0";
        model.filename = "Thranta-class_0002_veh_rep_destroyer_veh_rep_destroyer_metal01_d_0.geom";
        model.position = glm::vec3(0.f);
        model.scale = glm::vec3(1.f);
        model.rotation = glm::vec3(0.f);
        models.push_back(model);
    }
    {
        ModelDescription model{};
        model.id = startModelId + 3;
        model.name = "Thranta-class_0002_veh_rep_destroyer_veh_rep_destroyer_engine_d_0";
        model.filename = "Thranta-class_0002_veh_rep_destroyer_veh_rep_destroyer_engine_d_0.geom";
        model.position = glm::vec3(0.f);
        model.scale = glm::vec3(1.f);
        model.rotation = glm::vec3(0.f);
        models.push_back(model);
    }
    {
        ModelDescription model{};
        model.id = startModelId + 4;
        model.name = "Thranta-class_0002_veh_rep_destroyer_veh_rep_destroyer_trim_d_0";
        model.filename = "Thranta-class_0002_veh_rep_destroyer_veh_rep_destroyer_trim_d_0.geom";
        model.position = glm::vec3(0.f);
        model.scale = glm::vec3(1.f);
        model.rotation = glm::vec3(0.f);
        models.push_back(model);
    }

    return models;
}

std::vector<MaterialDescription> getThrantaMaterials(int startMaterialId)
{
    std::vector<MaterialDescription> materials;
    {
        MaterialDescription material{};
        material.id = startMaterialId + 0;
        material.name = "veh_rep_destroyer_detail_d";
        material.filename = "veh_rep_destroyer_detail_d.mats";
        material.albedo = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
        materials.push_back(material);
    }
    {
        MaterialDescription material{};
        material.id = startMaterialId + 1;
        material.name = "veh_rep_destroyer_engine_d";
        material.filename = "veh_rep_destroyer_engine_d.mats";
        material.albedo = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
        materials.push_back(material);
    }
    {
        MaterialDescription material{};
        material.id = startMaterialId + 2;
        material.name = "veh_rep_destroyer_metal01_d";
        material.filename = "veh_rep_destroyer_metal01_d.mats";
        material.albedo = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
        materials.push_back(material);
    }
    {
        MaterialDescription material{};
        material.id = startMaterialId + 3;
        material.name = "veh_rep_destroyer_metal02_d";
        material.filename = "veh_rep_destroyer_metal02_d.mats";
        material.albedo = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
        materials.push_back(material);
    }
    {
        MaterialDescription material{};
        material.id = startMaterialId + 4;
        material.name = "veh_rep_destroyer_trim_d";
        material.filename = "veh_rep_destroyer_trim_d.mats";
        material.albedo = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
        materials.push_back(material);
    }
    return materials;
}

GameObjectNode getThrantaParentNode(uint32_t& nodeId, uint32_t startModelId, uint32_t startMaterialId, const std::string& name,
                                    const glm::vec3& position, const glm::vec3& scale, const glm::vec3& rotation)
{
    GameObjectNode node{};
    node.id = nodeId++;
    node.name = name;
    node.position = position;
    node.scale = scale;
    node.rotation = rotation;
    {
        GameObjectNode object{};
        object.id = nodeId++;
        object.name = "Thranta-class_0002_veh_rep_destroyer_veh_rep_destroyer_metal02_d_0";
        object.modelId = startModelId + 0;
        object.materialId = startMaterialId + 3;
        object.position = glm::vec3(0.f);
        object.scale = glm::vec3(1.f);
        object.rotation = glm::vec3(0.f);
        node.children.push_back(object);
    }

    {
        GameObjectNode object{};
        object.id = nodeId++;
        object.name = "Thranta-class_0002_veh_rep_destroyer_veh_rep_destroyer_detail_d_0";
        object.modelId = startModelId + 1;
        object.materialId = startMaterialId + 0;
        object.position = glm::vec3(0.f);
        object.scale = glm::vec3(1.f);
        object.rotation = glm::vec3(0.f);
        node.children.push_back(object);
    }

    {
        GameObjectNode object{};
        object.id = nodeId++;
        object.name = "Thranta-class_0002_veh_rep_destroyer_veh_rep_destroyer_metal01_d_0";
        object.modelId = startModelId + 2;
        object.materialId = startMaterialId + 2;
        object.position = glm::vec3(0.f);
        object.scale = glm::vec3(1.f);
        object.rotation = glm::vec3(0.f);
        node.children.push_back(object);
    }

    {
        GameObjectNode object{};
        object.id = nodeId++;
        object.name = "Thranta-class_0002_veh_rep_destroyer_veh_rep_destroyer_engine_d_0";
        object.modelId = startModelId + 3;
        object.materialId = startMaterialId + 1;
        object.position = glm::vec3(0.f);
        object.scale = glm::vec3(1.f);
        object.rotation = glm::vec3(0.f);
        node.children.push_back(object);
    }

    {
        GameObjectNode object{};
        object.id = nodeId++;
        object.name = "Thranta-class_0002_veh_rep_destroyer_veh_rep_destroyer_trim_d_0";
        object.modelId = startModelId + 4;
        object.materialId = startMaterialId + 4;
        object.position = glm::vec3(0.f);
        object.scale = glm::vec3(1.f);
        object.rotation = glm::vec3(0.f);
        node.children.push_back(object);
    }

    return node;
}
