
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

static void loadModelFromFile(const std::string& filename)
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
    materials.push_back(tinyobj::material_t());
    printf("Number of materials: %zu\n", materials.size());
    // TODO: make all materials and register them into the material library so they can be used later
    // TODO: add material reference/id to the parts that are created here
    
    mats::MatsIO matsIo_ = mats::MatsIO();

    uint32_t materialId = 0u;
    //TODO: make material file format
     for(auto material : materials) {
        std::string matName = material.name.empty() ? "Default" : material.name;
        std::string materialFilename = matName  + ".mats";

        ++materialId;
         
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

        /*phong.ambient = materialData.ambient;
        phong.specular = materialData.specular;
        phong.shininess = materialData.shininess;*/

        //phong.ambientTextureSize = material.ambient_texname.size();
        /*phong.albedoTextureSize = material.diffuse_texname.size();
        phong.specularTextureSize = material.specular_texname.size();*/

        /*if (!material.ambient_texname.empty() && material.ambient_texname.size() <= 512)
        {
            memcpy(&phong.ambientTexture[0], material.ambient_texname.c_str(), std::fmin(512, material.ambient_texname.size()));
        }
        if (!material.diffuse_texname.empty() && material.diffuse_texname.size() <= 512) 
        {
            memcpy(&phong.albedoTexture[0], material.diffuse_texname.c_str(), std::fmin(512, material.diffuse_texname.size()));
        }
        if (!material.specular_texname.empty() && material.specular_texname.size() <= 512) 
        {
            memcpy(&phong.specularTexture[0], material.specular_texname.c_str(), std::fmin(512, material.specular_texname.size()));
        }*/


        printf("Saving material file %s\n", materialFilename.c_str());
        matsIo_.save(materialFilename.c_str(), mat);
     }
     

     geom::GeomIO geomIo_ = geom::GeomIO();

     std::string toc = "";

    // Go through shapes and create an indexed face set
    // TODO store a mesh in the model for each shape
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

        toc += "Shape: " + shape.name + "\n";

        int meshIndex = 0;
        for (auto& vert : modelVertices) {
            int matId = vert.first;
            printf("Creating buffers for material: %i\n", matId);
            auto& vertData = vert.second;


            auto& indData = modelFaces.at(matId);

            std::string meshName = shape.name + "_" + std::to_string(meshIndex);
            std::string meshFilename = meshName + ".geom";

            toc += "\t{\n\t\tMesh: " + meshName + "\n";
            toc += "\t\tMaterial: " + materials[matId].name + "\n\t}\n";

            printf("Saving meshfile %s\n", meshFilename.c_str());

            
            geomIo_.save(meshFilename.c_str(), (float*)vertData.data(), vertData.size()*8, (uint32_t*)indData.data(), indData.size()*3);

            ++meshIndex;
        }
        toc += "\n";
    }

    std::string tocFilename = filepath + "_toc.txt";
    printf("Saving table of contents file %s\n", tocFilename.c_str());
    std::FILE* tocFile = std::fopen(tocFilename.c_str(), "w");
    if (!tocFile) {
        printf("Failed to create table of contents file %s\n", tocFilename.c_str());
        return;
    }

    std::fwrite(toc.c_str(), 1, toc.size(), tocFile);
    std::fflush(tocFile);
    std::fclose(tocFile);

}

int main(int argc, char** argv) 
{
    if(argc <= 1) {
        printf("Usage: model_maker.exe filename.obj");
        return 0;
    }
    else {
        std::string objFile = std::string(argv[1]);
        loadModelFromFile(objFile);
        return 0;
    }
}