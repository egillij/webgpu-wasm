#pragma once

#include <string>
#include <vector>

class WGpuVertexBuffer;
class WGpuIndexBuffer;
class WGpuDevice;

struct MeshPart {
    WGpuVertexBuffer* vertexBuffer;
    WGpuIndexBuffer* indexBuffer;

    bool isReady() const { return vertexBuffer != nullptr && indexBuffer != nullptr; }
};

class TriangleMesh {
public:
    TriangleMesh(const std::string& name);
    ~TriangleMesh();

    void update(const float* vertices, size_t verticesSize, const uint32_t* indices, uint32_t indexCount, WGpuDevice* device);

    inline bool isReady() const { return m_VertexBuffer != nullptr && m_IndexBuffer != nullptr; }
    inline WGpuVertexBuffer* getVertexBuffer() const {return m_VertexBuffer;}
    inline WGpuIndexBuffer* getIndexBuffer() const { return m_IndexBuffer; }

private:
    std::string m_Name;
    
    WGpuVertexBuffer* m_VertexBuffer;
    WGpuIndexBuffer* m_IndexBuffer;
};