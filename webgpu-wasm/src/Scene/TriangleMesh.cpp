#include "TriangleMesh.h"

#include "ModelLoader.h"

#include "Renderer/WebGPU/wgpuVertexBuffer.h"
#include "Renderer/WebGPU/wgpuIndexBuffer.h"
#include "Renderer/WebGPU/wgpuDevice.h"

#include <emscripten.h>

TriangleMesh::TriangleMesh(const std::string& name)
: m_Name(name), m_VertexBuffer(nullptr), m_IndexBuffer(nullptr)
{
}

TriangleMesh::~TriangleMesh()
{
    if(m_VertexBuffer) delete m_VertexBuffer;
    if(m_IndexBuffer) delete m_IndexBuffer;
}

void TriangleMesh::loadFromFile(const std::string& filename, WGpuDevice* device)
{
    std::string m_ServerResource = "/webgpu-wasm/resources/models/" + filename;
    std::string m_LocalResource = "./models/" + filename;
    emscripten_wget(m_ServerResource.c_str(), m_LocalResource.c_str());
    ModelData model = ModelLoader::loadModelFromFile(m_LocalResource.c_str());

    // For now we just load the first mesh from the .obj file. Later model loading should be moved out of this and create as many triangle meshes as needed for the file
    m_VertexBuffer = new WGpuVertexBuffer(device, m_Name + "- Vertex Buffer", model.modelData.at(0).vertexData, model.modelData.at(0).numberOfVertices*8*sizeof(float));
    m_IndexBuffer = new WGpuIndexBuffer(device, m_Name + "- Index Buffer", model.modelData.at(0).indexData, model.modelData.at(0).numberOfIndices, IndexBufferFormat::UNSIGNED_INT_32);
}