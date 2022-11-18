#pragma once

#include "Scene/Scene.h"

void getJediStarfighterParts(int startModelId, const std::string& folder, std::vector<ModelDescription>& models)
{
	{
		ModelDescription model{};
		model.id = startModelId + 0;
		model.name = "main_mainShape_jsf_mainMat_0";
		model.filename = folder + "/main_mainShape_jsf_mainMat_0.geom";
		model.position = glm::vec3(0.f);
		model.scale = glm::vec3(1.f);
		model.rotation = glm::vec3(0.f); 
		models.push_back(model);
	}
	{
		ModelDescription model{};
		model.id = startModelId + 1;
		model.name = "aux_auxShape_jsf_auxMat_0";
		model.filename = folder + "/aux_auxShape_jsf_auxMat_0.geom";
		model.position = glm::vec3(0.f);
		model.scale = glm::vec3(1.f);
		model.rotation = glm::vec3(0.f); 
		models.push_back(model);
	}
}

void getJediStarfighterMaterials(int startMaterialId, const std::string& folder, std::vector<MaterialDescription>& materials)
{
	{
		MaterialDescription material{};
		material.id = startMaterialId + 0;
		material.name = "jsf_auxMat";
		material.filename = folder + "/jsf_auxMat.mats";
		material.albedo = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		materials.push_back(material);
	}
	{
		MaterialDescription material{};
		material.id = startMaterialId + 1;
		material.name = "jsf_mainMat";
		material.filename = folder + "/jsf_mainMat.mats";
		material.albedo = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		materials.push_back(material);
	}
}

GameObjectNode getJediStarfighterParentNode(uint32_t& nodeId, uint32_t startModelId, uint32_t startMaterialId, const std::string& name, const glm::vec3 & position, const glm::vec3& scale, const glm::vec3& rotation)
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
		object.name = "main_mainShape_jsf_mainMat_0";
		object.modelId = startModelId + 0;
		object.materialId = startMaterialId + 1;
		object.position = glm::vec3(0.f);
		object.scale = glm::vec3(1.f);
		object.rotation = glm::vec3(0.f);
		node.children.push_back(object);
	}

	{
		GameObjectNode object{};
		object.id = nodeId++;
		object.name = "aux_auxShape_jsf_auxMat_0";
		object.modelId = startModelId + 1;
		object.materialId = startMaterialId + 0;
		object.position = glm::vec3(0.f);
		object.scale = glm::vec3(1.f);
		object.rotation = glm::vec3(0.f);
		node.children.push_back(object);
	}

	return node;
}

