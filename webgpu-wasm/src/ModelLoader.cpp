#include "ModelLoader.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>

#include <emscripten.h>

namespace std {
    // inline bool operator<(const tinyobj::index_t& a,
    //     const tinyobj::index_t& b)
    // {
    //     if (a.vertex_index < b.vertex_index) return true;
    //     if (a.vertex_index > b.vertex_index) return false;

    //     if (a.normal_index < b.normal_index) return true;
    //     if (a.normal_index > b.normal_index) return false;

    //     if (a.texcoord_index < b.texcoord_index) return true;
    //     if (a.texcoord_index > b.texcoord_index) return false;

    //     return false;
    // }

    // inline bool operator<(const tinyobj::index_t a,
    //     const tinyobj::index_t b)
    // {
    //     if (a.vertex_index < b.vertex_index) return true;
    //     if (a.vertex_index > b.vertex_index) return false;

    //     if (a.normal_index < b.normal_index) return true;
    //     if (a.normal_index > b.normal_index) return false;

    //     if (a.texcoord_index < b.texcoord_index) return true;
    //     if (a.texcoord_index > b.texcoord_index) return false;

    //     return false;
    // }
}

struct Vertex {
    float position[3] = {0.f, 0.f, 0.f};
    float normal[3] = {0.f, 0.f, 0.f};
    float uv[2] = {0.f, 0.f};
};

struct Face {
    uint32_t indices[3];
};

ModelData ModelLoader::loadModelFromFile(const char* filename)
{
    ModelData modelData;

    //TINYOBJLOADER
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string filepath = std::string(filename);
    size_t lastSlash = filepath.find_last_of('/');
    std::string base_dir = filepath.substr(0, lastSlash+1);
    std::string warn;
    std::string err;
    bool objRead;
    {
        objRead = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str(), base_dir.c_str());
    }

    if (!warn.empty()) {
        emscripten_log(EM_LOG_WARN, "Warning during tinyobj::LoadOBJ : %s\n", warn.c_str());
    }
    if (!err.empty()) {
        emscripten_log(EM_LOG_ERROR, "Error during tinyobj::LoadOBJ : %s\n", err.c_str());
    }
    if (!objRead)
        return modelData;

    // Append `default` material
    materials.push_back(tinyobj::material_t());

    //TODO: make all materials and register them into the material library so they can be read during OBJ processing
    // for (auto material : materials)
    // {
    //     if (!MaterialLibrary::Exists(material.name))
    //     {
    //         glm::vec4 ambientColor = { material.ambient[0], material.ambient[1], material.ambient[2], 1.f };
    //         glm::vec4 diffuseColor = { material.diffuse[0], material.diffuse[1], material.diffuse[2], 1.f };
    //         glm::vec4 specularColor = { material.specular[0], material.specular[1], material.specular[2], 1.f };

    //         Ref<Material> mat = CreateRef<Material>(material.name,
    //             ambientColor, material.ambient_texname,
    //             diffuseColor, material.diffuse_texname,
    //             specularColor, material.specular_texname, material.shininess,
    //             material.bump_texopt.bump_multiplier, material.bump_texname,
    //             base_dir);
    //         MaterialLibrary::Add(material.name, mat);
    //     }
        
    // }

    //Go through shapes and create an indexed face set
    //TODO store a mesh in the model for each shape
    
    for (auto shape : shapes)
    {
        auto objMesh = shape.mesh;
        unsigned int i = 0;
        unsigned int offset = 0;

        std::vector<Vertex> modelVertices;
        std::vector<Face> modelFaces;


        std::map<tinyobj::index_t, int> knownVertices;
        while (true)
        {
            // Expect this to be 3 for triangles
            int faceSize = objMesh.num_face_vertices[i];
            
            //std::vector<tinyobj::index_t> faceIndices;
            //faceIndices.resize(faceSize);

            std::vector<uint32_t> indexList;
            int faceIndices[3] = { -1, -1, -1 };
            for (int j = 0; j < faceSize; j++)
            {
                auto vertexIndices = objMesh.indices[j+offset];
                auto found = knownVertices.find(vertexIndices);                    
                
                if (found == knownVertices.end())
                {
                    knownVertices[vertexIndices] = modelVertices.size();

                    std::vector<float> vertexPosition(&attrib.vertices[vertexIndices.vertex_index * 3], &attrib.vertices[vertexIndices.vertex_index * 3] + 3);

                    Vertex faceVertex{};
                    faceVertex.position[0] = vertexPosition[0];
                    faceVertex.position[1] = vertexPosition[1];
                    faceVertex.position[2] = vertexPosition[2];

                    if (vertexIndices.normal_index != -1) {
                        std::vector<float> vertexNormal(&attrib.normals[vertexIndices.normal_index * 3], &attrib.normals[vertexIndices.normal_index * 3] + 3);
                        faceVertex.normal[0] = vertexNormal[0];
                        faceVertex.normal[1] = vertexNormal[1];
                        faceVertex.normal[2] = vertexNormal[2];
                    }
                    else {
                        //TODO: calculate face normal from positions
                    }

                    if (vertexIndices.texcoord_index != -1) {
                        std::vector<float> vertexTexCoord(&attrib.texcoords[vertexIndices.texcoord_index * 2], &attrib.texcoords[vertexIndices.texcoord_index * 2] + 2);
                        faceVertex.uv[0] = vertexTexCoord[0];
                        faceVertex.uv[0] = vertexTexCoord[1];
                    }

                    modelVertices.push_back(faceVertex);
                    indexList.push_back(modelVertices.size() - 1);
                }
                else 
                {
                    indexList.push_back(found->second);
                    
                }
            }
            
            modelFaces.push_back({indexList[0], indexList[1], indexList[2]});
            
            offset += faceSize;
            
            i += 1;

            if (i >= objMesh.num_face_vertices.size())
                break;
        }

        ModelData::PartData part{};
        part.vertexData = (float*) malloc(modelVertices.size()*8*sizeof(float));
        memcpy((void*)part.vertexData, (void*)modelVertices.data(), modelVertices.size()*sizeof(Vertex));
        part.numberOfVertices = modelVertices.size();

        part.indexData = (uint32_t*)malloc(modelFaces.size() * 3 * sizeof(uint32_t));
        memcpy((void*)part.indexData, (void*)modelFaces.data(), modelFaces.size()*sizeof(Face));
        part.numberOfIndices = modelFaces.size() * 3;

        modelData.modelData.push_back(part);
    }

    return modelData;
}