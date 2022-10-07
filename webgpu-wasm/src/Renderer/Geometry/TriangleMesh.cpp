#include "TriangleMesh.h"

#include "ModelLoader.h"

#include "Renderer/WebGPU/wgpuVertexBuffer.h"
#include "Renderer/WebGPU/wgpuIndexBuffer.h"
#include "Renderer/WebGPU/wgpuDevice.h"

#include <emscripten.h>

TriangleMesh::TriangleMesh(const std::string &name)
    : m_Name(name)
{
    m_Parts.clear();
}

TriangleMesh::~TriangleMesh()
{
    for(auto& part: m_Parts){
    if (part.vertexBuffer)
        delete part.vertexBuffer;
    if (part.indexBuffer)
        delete part.indexBuffer;
    }
    
}

void TriangleMesh::loadFromFile(const std::string &filename, WGpuDevice *device)
{
    std::string m_ServerResource = "/resources/models/" + filename;
    std::string m_LocalResource = "./models/" + filename;
    emscripten_wget(m_ServerResource.c_str(), m_LocalResource.c_str());

    std::string m_ServerMtl = m_ServerResource.substr(0, m_ServerResource.size() - 3) + "mtl";
    std::string m_LocalMtl = m_LocalResource.substr(0, m_LocalResource.size() - 3) + "mtl";
    emscripten_wget(m_ServerMtl.c_str(), m_LocalMtl.c_str());

    ModelData model = ModelLoader::loadModelFromFile(m_LocalResource.c_str());

    for (int i = 0; i < model.modelData.size(); ++i)
    {
        m_Parts.push_back({nullptr, nullptr});
        MeshPart& part = m_Parts.back();

        // For now we just load the first mesh from the .obj file. Later model loading should be moved out of this and create as many triangle meshes as needed for the file
        part.vertexBuffer = new WGpuVertexBuffer(device, m_Name + + "- Vertex Buffer_" + std::to_string(i), model.modelData.at(i).vertexData, model.modelData.at(i).numberOfVertices * 8 * sizeof(float));
        part.indexBuffer = new WGpuIndexBuffer(device, m_Name + "- Index Buffer_" + std::to_string(i), model.modelData.at(i).indexData, model.modelData.at(i).numberOfIndices, IndexBufferFormat::UNSIGNED_INT_32);

        // Clean up 
        if (model.modelData.at(i).indexData)
            free(model.modelData.at(i).indexData);

        if (model.modelData.at(i).vertexData)
            free(model.modelData.at(i).vertexData);
    }
}