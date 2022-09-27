#pragma once

#include <string>

class WGpuVertexBuffer;
class WGpuIndexBuffer;
class WGpuDevice;

class TriangleMesh {
public:
    TriangleMesh(const std::string& name);
    ~TriangleMesh();

    void loadFromFile(const std::string& filename, WGpuDevice* device);

    bool isReady() const { return m_VertexBuffer != nullptr && m_IndexBuffer != nullptr; }

    inline WGpuVertexBuffer* getVertexBuffer() const {return m_VertexBuffer;}
    inline WGpuIndexBuffer* getIndexBuffer() const { return m_IndexBuffer; }

private:
    std::string m_Name;
    WGpuVertexBuffer* m_VertexBuffer;
    WGpuIndexBuffer* m_IndexBuffer;

    // Resource location
    std::string m_ServerResource;
    std::string m_LocalResource;
};