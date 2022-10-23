#include "TriangleMesh.h"

#include "Renderer/WebGPU/wgpuVertexBuffer.h"
#include "Renderer/WebGPU/wgpuIndexBuffer.h"
#include "Renderer/WebGPU/wgpuDevice.h"

#include <emscripten.h>

TriangleMesh::TriangleMesh(const std::string &name)
    : m_Name(name), m_VertexBuffer(nullptr), m_IndexBuffer(nullptr)
{
}

TriangleMesh::~TriangleMesh()
{
    if(m_VertexBuffer)
        delete m_VertexBuffer;

    if(m_IndexBuffer)
        delete m_IndexBuffer;
}

void TriangleMesh::update(const float* vertices, size_t verticesSize, const uint32_t* indices, uint32_t indexCount, WGpuDevice* device)
{
    m_VertexBuffer = new WGpuVertexBuffer(device, m_Name + " - Vertex Buffer", (void*)vertices, verticesSize);
    m_IndexBuffer = new WGpuIndexBuffer(device, m_Name + " - Index Buffer", (void*)indices, indexCount, IndexBufferFormat::UNSIGNED_INT_32);
}
