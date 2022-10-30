
#define GEOM_IO_SAVE
#include "geomIo.h"

#define MATS_IO_SAVE
#include "matsIo.h"

#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <string>

#include <cstdio>
#include <vector>


struct Vertex
{
    float position[3] = {0.f, 0.f, 0.f};
    float normal[3] = {0.f, 0.f, 0.f};
    float uv[2] = {0.f, 0.f};
};

struct Face
{
    uint32_t indices[3];
};

static void loadModelFromFile(const std::string& filename, const std::string& modelName)
{

    // TINYOBJLOADER
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string filepath = filename;
    size_t lastSlash = filepath.find_last_of('/');
    std::string base_dir = filepath.substr(0, lastSlash + 1);
    std::string warn;
    std::string err;
    printf("Loading OBJ file %s...\n", filepath.c_str());
    bool objRead = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str(), base_dir.c_str());

    if (!warn.empty())
    {
        printf("Warning during tinyobj::LoadOBJ : %s\n", warn.c_str());
    }
    if (!err.empty())
    {
        printf("Error during tinyobj::LoadOBJ : %s\n", err.c_str());
    }
    if (!objRead){
        printf("Failed to read OBJ file %s\n", filename.c_str());
        return;
    }

    // Append `default` material
    //materials.push_back(tinyobj::material_t());
    printf("Number of materials: %zu\n", materials.size());
    // TODO: make all materials and register them into the material library so they can be used later
    // TODO: add material reference/id to the parts that are created here
    
    std::string tocMaterial = "void get" + modelName + "Materials(int startMaterialId, std::vector<MaterialDescription>& materials)\n{\n";

    mats::MatsIO matsIo_ = mats::MatsIO();

    uint32_t materialId = 0u;
    //TODO: make material file format

    std::unordered_map<std::string, uint32_t> materialIdMap;

     for(auto material : materials) {
        std::string matName = material.name.empty() ? "Default" : material.name;
        std::string materialFilename = matName  + ".mats";
        
        materialIdMap[matName] = materialId;
         
        mats::MaterialParameters mat{};
        memset(&mat, 0, sizeof(mat));

        mat.albedo = { material.diffuse[0], material.diffuse[1], material.diffuse[2] };
        mat.metallic = material.metallic;
        mat.roughness = material.roughness;
        mat.ao = (material.ambient[0] + material.ambient[1] + material.ambient[2]) / 3.f;

        mat.albedoTextureSize = material.diffuse_texname.size();
        mat.metallicTextureSize = material.metallic_texname.size();
        mat.roughnessTextureSize = material.roughness_texname.size();
        mat.aoTextureSize = material.ambient_texname.size();

        if (!material.diffuse_texname.empty() && material.diffuse_texname.size() <= 512)
        {
            memcpy(&mat.albedoTexture[0], material.diffuse_texname.c_str(), std::fmin(512, material.diffuse_texname.size()));
        }

        if (!material.metallic_texname.empty() && material.metallic_texname.size() <= 512)
        {
            memcpy(&mat.metallicTexture[0], material.metallic_texname.c_str(), std::fmin(512, material.metallic_texname.size()));
        }

        if (!material.roughness_texname.empty() && material.roughness_texname.size() <= 512)
        {
            memcpy(&mat.roughnessTexture[0], material.roughness_texname.c_str(), std::fmin(512, material.roughness_texname.size()));
        }

        if (!material.ambient_texname.empty() && material.ambient_texname.size() <= 512)
        {
            memcpy(&mat.aoTexture[0], material.ambient_texname.c_str(), std::fmin(512, material.ambient_texname.size()));
        }

        printf("Saving material file %s\n", materialFilename.c_str());
        matsIo_.save(materialFilename.c_str(), mat);

        tocMaterial += "\t{\n\t\tMaterialDescription material{};\n";
        tocMaterial += "\t\tmaterial.id = startMaterialId + " + std::to_string(materialId) + ";\n";
        tocMaterial += "\t\tmaterial.name = \"" + material.name + "\";\n";
        tocMaterial += "\t\tmaterial.filename = \"" + materialFilename + "\";\n";
        tocMaterial += "\t\tmaterial.albedo = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);\n";
        tocMaterial += "\t\tmaterials.push_back(material);\n\t}\n";

        ++materialId;
     }
     

     geom::GeomIO geomIo_ = geom::GeomIO();

     std::string toc = "";
     std::string tocGeo = "void get" + modelName + "Parts(int startModelId, std::vector<ModelDescription>& models)\n{\n";

     toc += "GameObjectNode get" + modelName + "ParentNode(uint32_t& nodeId, uint32_t startModelId, uint32_t startMaterialId, const std::string& name, const glm::vec3 & position, const glm::vec3& scale, const glm::vec3& rotation)\n{\n";
     toc += "\tGameObjectNode node{};\n";
     toc += "\tnode.id = nodeId++;\n";
     toc += "\tnode.name = name;\n";
     toc += "\tnode.position = position;\n";
     toc += "\tnode.scale = scale;\n";
     toc += "\tnode.rotation = rotation;\n";

    // Go through shapes and create an indexed face set
    // TODO store a mesh in the model for each shape
     int shapeIndex = 0;
    for (auto shape : shapes)
    {
        auto objMesh = shape.mesh;
        unsigned int i = 0;
        unsigned int offset = 0;

        std::unordered_map<int, std::vector<Vertex>> modelVertices;
        std::unordered_map<int, std::vector<Face>> modelFaces;

        std::unordered_map<int, std::map<tinyobj::index_t, int>> knownVertices;
        while (true)
        {
            // Expect this to be 3 for triangles
            int faceSize = objMesh.num_face_vertices[i];

            // std::vector<tinyobj::index_t> faceIndices;
            // faceIndices.resize(faceSize);

            std::vector<uint32_t> indexList;
            int faceIndices[3] = {-1, -1, -1};
            int materialId = objMesh.material_ids[i];

            for (int j = 0; j < faceSize; j++)
            {
                auto vertexIndices = objMesh.indices[j + offset];
                auto matIt = knownVertices.find(materialId);
                if(matIt == knownVertices.end()){ 
                    knownVertices[materialId] = {};
                    matIt = knownVertices.find(materialId);
                }

                auto it = modelVertices.find(materialId);
                if(it == modelVertices.end()) modelVertices[materialId] = {};

                auto found = matIt->second.find(vertexIndices);

                if (found == matIt->second.end())
                {
                    knownVertices[materialId][vertexIndices] = modelVertices.at(materialId).size();

                    std::vector<float> vertexPosition(&attrib.vertices[vertexIndices.vertex_index * 3], &attrib.vertices[vertexIndices.vertex_index * 3] + 3);

                    Vertex faceVertex{};
                    faceVertex.position[0] = vertexPosition[0];
                    faceVertex.position[1] = vertexPosition[1];
                    faceVertex.position[2] = vertexPosition[2];

                    if (vertexIndices.normal_index != -1)
                    {
                        std::vector<float> vertexNormal(&attrib.normals[vertexIndices.normal_index * 3], &attrib.normals[vertexIndices.normal_index * 3] + 3);
                        faceVertex.normal[0] = vertexNormal[0];
                        faceVertex.normal[1] = vertexNormal[1];
                        faceVertex.normal[2] = vertexNormal[2];
                    }
                    else
                    {
                        // TODO: calculate face normal from positions
                    }

                    if (vertexIndices.texcoord_index != -1)
                    {
                        std::vector<float> vertexTexCoord(&attrib.texcoords[vertexIndices.texcoord_index * 2], &attrib.texcoords[vertexIndices.texcoord_index * 2] + 2);
                        faceVertex.uv[0] = vertexTexCoord[0];
                        faceVertex.uv[1] = vertexTexCoord[1];
                    }
                    

                    modelVertices.at(materialId).push_back(faceVertex);
                    indexList.push_back(modelVertices.at(materialId).size() - 1);
                }
                else
                {
                    indexList.push_back(found->second);
                }
            }
            
            auto it = modelFaces.find(materialId);
            if(it == modelFaces.end()) modelFaces[materialId] = {};
            modelFaces.at(materialId).push_back({indexList[0], indexList[1], indexList[2]});

            offset += faceSize;

            i += 1;

            if (i >= objMesh.num_face_vertices.size())
                break;
        }

        //toc += "Shape: " + shape.name + "\n";

        int meshIndex = 0;
        for (auto& vert : modelVertices) {
            int matId = vert.first;
            printf("Creating buffers for material: %i\n", matId);
            auto& vertData = vert.second;


            auto& indData = modelFaces.at(matId);

            std::string meshName = shape.name + "_" + std::to_string(meshIndex);
            std::string meshFilename = meshName + ".geom";

            tocGeo += "\t{\n\t\tModelDescription model{};\n";
            tocGeo += "\t\tmodel.id = startModelId + " + std::to_string(shapeIndex) + ";\n";
            tocGeo += "\t\tmodel.name = \"" + meshName + "\";\n";
            tocGeo += "\t\tmodel.filename = \"" + meshFilename + "\";\n";
            tocGeo += "\t\tmodel.position = glm::vec3(0.f);\n";
            tocGeo += "\t\tmodel.scale = glm::vec3(1.f);\n";
            tocGeo += "\t\tmodel.rotation = glm::vec3(0.f); \n";
            tocGeo += "\t\tmodels.push_back(model);\n\t}\n";

            toc += "\t{\n\t\tGameObjectNode object{};\n";
            toc += "\t\tobject.id = nodeId++;\n";
            toc += "\t\tobject.name = \"" + meshName + "\";\n";
            toc += "\t\tobject.modelId = startModelId + " + std::to_string(shapeIndex) + ";\n";
            toc += "\t\tobject.materialId = startMaterialId + " + std::to_string(matId) + ";\n";// INSERT_MATERIAL_ID; \n";
            toc += "\t\tobject.position = glm::vec3(0.f);\n";
            toc += "\t\tobject.scale = glm::vec3(1.f);\n";
            toc += "\t\tobject.rotation = glm::vec3(0.f);\n";
            toc += "\t\tnode.children.push_back(object);\n\t}\n";

            /*toc += "\t{\n\t\tMesh: " + meshFilename + "\n";
            toc += "\t\tMaterial: " + materials[matId].name + ".mats\n\t}\n";*/

            printf("Saving meshfile %s\n", meshFilename.c_str());

            
            geomIo_.save(meshFilename.c_str(), (float*)vertData.data(), vertData.size()*8, (uint32_t*)indData.data(), indData.size()*3);

            ++meshIndex;
            ++shapeIndex;
        }
        toc += "\n";
    }
    
    tocMaterial += "}\n";
    tocGeo += "}\n";
    toc += "\treturn node;\n}\n";

    std::string modelHeaderFile = modelName + ".h";
    printf("Saving model .h file %s\n", modelHeaderFile.c_str());
    std::FILE* fp = std::fopen(modelHeaderFile.c_str(), "w");
    if (!fp) {
        printf("Failed to create table of contents file %s\n", modelHeaderFile.c_str());
        return;
    }

    std::string headerContents = "#pragma once\n\n#include \"Scene/Scene.h\"\n\n";

    headerContents += tocGeo + "\n";
    headerContents += tocMaterial + "\n";
    headerContents += toc + "\n";
    
    std::fwrite(headerContents.c_str(), 1, headerContents.size(), fp);
    std::fflush(fp);
    std::fclose(fp);

}

int main(int argc, char** argv) 
{
    if(argc <= 1) {
        printf("Usage: model_maker.exe filename.obj ModelName");
        return 0;
    }
    else if (argc <= 2) {
        printf("Missing argument:\n");
        printf("Usage: model_maker.exe filename.obj ModelName");
    }
    else {
        std::string objFile = std::string(argv[1]);
        std::string modelName = std::string(argv[2]);
        loadModelFromFile(objFile, modelName);
        return 0;
    }
}
