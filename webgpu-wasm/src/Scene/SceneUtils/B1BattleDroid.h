#pragma once

#include "Scene/Scene.h"

void getB1BattleDroidParts(int startModelId, std::vector<ModelDescription>& models)
{
	{
		ModelDescription model{};
		model.id = startModelId + 0;
		model.name = "Mesh51.001_0";
		model.filename = "Mesh51.001_0.geom";
		model.position = glm::vec3(0.f);
		model.scale = glm::vec3(1.f);
		model.rotation = glm::vec3(0.f); 
		models.push_back(model);
	}
	{
		ModelDescription model{};
		model.id = startModelId + 1;
		model.name = "Mesh51.001_1";
		model.filename = "Mesh51.001_1.geom";
		model.position = glm::vec3(0.f);
		model.scale = glm::vec3(1.f);
		model.rotation = glm::vec3(0.f); 
		models.push_back(model);
	}
}

void getB1BattleDroidMaterials(int startMaterialId, std::vector<MaterialDescription>& materials)
{
	{
		MaterialDescription material{};
		material.id = startMaterialId + 0;
		material.name = "01___Def";
		material.filename = "01___Def.mats";
		material.albedo = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		materials.push_back(material);
	}
	{
		MaterialDescription material{};
		material.id = startMaterialId + 1;
		material.name = "07___Def";
		material.filename = "07___Def.mats";
		material.albedo = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		materials.push_back(material);
	}
}

GameObjectNode getB1BattleDroidParentNode(uint32_t& nodeId, uint32_t startModelId, uint32_t startMaterialId, const std::string& name, const glm::vec3 & position, const glm::vec3& scale, const glm::vec3& rotation)
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
		object.name = "Mesh51.001_0";
		object.modelId = startModelId + 0;
		object.materialId = startMaterialId + 0;
		object.position = glm::vec3(0.f);
		object.scale = glm::vec3(1.f);
		object.rotation = glm::vec3(0.f);
		node.children.push_back(object);
	}
	{
		GameObjectNode object{};
		object.id = nodeId++;
		object.name = "Mesh51.001_1";
		object.modelId = startModelId + 1;
		object.materialId = startMaterialId + 1;
		object.position = glm::vec3(0.f);
		object.scale = glm::vec3(1.f);
		object.rotation = glm::vec3(0.f);
		node.children.push_back(object);
	}

	return node;
}

