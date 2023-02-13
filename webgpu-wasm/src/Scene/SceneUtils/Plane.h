// Copyright 2023 Egill Ingi Jacobsen

#pragma once

#include "Scene/Scene.h"

void getPlaneParts(int startModelId, const std::string& folder, std::vector<ModelDescription>& models)
{
	{
		ModelDescription model{};
		model.id = startModelId + 0;
		model.name = "Plane_0";
		model.filename = folder + "/Plane_0.geom";
		model.position = glm::vec3(0.f);
		model.scale = glm::vec3(1.f);
		model.rotation = glm::vec3(0.f); 
		models.push_back(model);
	}
}

void getPlaneMaterials(int startMaterialId, const std::string& folder, std::vector<MaterialDescription>& materials)
{
	{
		MaterialDescription material{};
		material.id = startMaterialId + 0;
		material.name = "PlaneMaterial";
		material.filename = folder + "/PlaneMaterial.mats";
		material.albedo = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		materials.push_back(material);
	}
}

GameObjectNode getPlaneParentNode(uint32_t& nodeId, uint32_t startModelId, uint32_t startMaterialId, const std::string& name, const glm::vec3 & position, const glm::vec3& scale, const glm::vec3& rotation)
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
		object.name = "Plane_0";
		object.modelId = startModelId + 0;
		object.materialId = startMaterialId + 0;
		object.position = glm::vec3(0.f);
		object.scale = glm::vec3(1.f);
		object.rotation = glm::vec3(0.f);
		node.children.push_back(object);
	}

	return node;
}

