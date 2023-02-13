// Copyright 2023 Egill Ingi Jacobsen

#pragma once

#include <string>
#include <vector>

class WGpuVertexBuffer;
class WGpuIndexBuffer;
class WGpuDevice;

class TriangleMesh {
public:
    TriangleMesh(const uint32_t id, const std::string& name);
    ~TriangleMesh();

    void update(const float* vertices, size_t verticesSize, const uint32_t* indices, uint32_t indexCount, WGpuDevice* device);

    inline bool isReady() const { return m_VertexBuffer != nullptr && m_IndexBuffer != nullptr; }
    inline WGpuVertexBuffer* getVertexBuffer() const {return m_VertexBuffer;}
    inline WGpuIndexBuffer* getIndexBuffer() const { return m_IndexBuffer; }

    inline uint32_t getId() const { return m_Id; }

private:
    uint32_t m_Id;
    std::string m_Name;
    
    WGpuVertexBuffer* m_VertexBuffer;
    WGpuIndexBuffer* m_IndexBuffer;
};