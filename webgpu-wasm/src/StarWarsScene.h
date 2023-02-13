// // Copyright 2023 Egill Ingi Jacobsen

// #pragma once

// static void StarWarsScene()
// {
//     const char *battleDroidFile = "b1_battle_droid.obj"; //"character.obj";

//     SceneDescription scene{};
//     scene.name = "Test Scene";
//     std::vector<ModelDescription> models;

//     {
//         ModelDescription model{};
//         model.id = 1;
//         model.name = "Battle Droid";
//         model.filename = "Mesh51.001_0.geom";
//         model.position = glm::vec3(0.f);
//         model.scale = glm::vec3(1.f);
//         model.rotation = glm::vec3(0.f);
//         models.push_back(model);
//     }
//     {
//         ModelDescription model{};
//         model.id = 2;
//         model.name = "Blaster";
//         model.filename = "Mesh51.001_1.geom";
//         model.position = glm::vec3(0.f);
//         model.scale = glm::vec3(1.f);
//         model.rotation = glm::vec3(0.f);
//         models.push_back(model);
//     }

//     std::vector<MaterialDescription> materials;
//     {
//         MaterialDescription material{};
//         material.id = 1;
//         material.name = "Droid Material";
//         material.filename = "01___Def.mats";
//         material.albedo = glm::vec4(1.0f, 0.0f, 0.0f, 1.f);
//         materials.push_back(material);
//     }
//     {
//         MaterialDescription material{};
//         material.id = 2;
//         material.name = "Blaster Material";
//         material.filename = "07___Def.mats";
//         material.albedo = glm::vec4(0.0f, 0.0f, 1.0f, 1.f);
//         materials.push_back(material);
//     }

//     scene.modelDescriptions = models.data();
//     scene.numberOfModels = models.size();

//     scene.materialDescriptons = materials.data();
//     scene.numberOfMaterials = materials.size();

//     std::vector<GameObjectNode> gameObjects;

//     int xFactor = 50;
//     int zFactor = 50;
//     float xOffset = float(xFactor) / 2.f;
//     float zOffset = float(zFactor) / 2.f;

//     for (int x = 0; x < xFactor; ++x)
//     {
//         for (int z = 0; z < zFactor; ++z)
//         {
//             {
//                 GameObjectNode object{};
//                 object.id = x*zFactor+z;
//                 object.name = "Droid " + std::to_string(object.id);
//                 object.modelId = 1;
//                 object.materialId = 1;
//                 object.position = glm::vec3(float(x) - xOffset, 0.f, -float(z));
//                 object.scale = glm::vec3(1.f);
//                 object.rotation = glm::vec3(0.f);
//                 gameObjects.push_back(object);
//             }
            
//             {
//                 GameObjectNode object{};
//                 object.id = x*zFactor+z;
//                 object.name = "Blaster " + std::to_string(object.id);
//                 object.modelId = 2;
//                 object.materialId = 2;
//                 object.position = glm::vec3(float(x) - xOffset, 0.f, -float(z));
//                 object.scale = glm::vec3(1.f);
//                 object.rotation = glm::vec3(0.f);
//                 gameObjects.push_back(object);
//             }
            
//         }
//     }
//     scene.gameObjects = gameObjects.data();
//     scene.numberOfGameObjects = gameObjects.size();
// }