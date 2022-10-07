#pragma once

#include <string>
#include <vector>

class WGpuVertexBuffer;
class WGpuIndexBuffer;
class WGpuDevice;

struct Part {
    WGpuVertexBuffer* vertexBuffer;
    WGpuIndexBuffer* indexBuffer;

    bool isReady() const { return vertexBuffer != nullptr && indexBuffer != nullptr; }
};

class TriangleMesh {
public:
    TriangleMesh(const std::string& name);
    ~TriangleMesh();

    void loadFromFile(const std::string& filename, WGpuDevice* device);

    inline size_t getNumberOfParts() const {return m_Parts.size();}
    inline bool isPartReady(uint32_t index) const { return m_Parts.at(index).isReady(); }
    inline WGpuVertexBuffer* getPartVertexBuffer(uint32_t index) const {return m_Parts.at(index).vertexBuffer;}
    inline WGpuIndexBuffer* getPartIndexBuffer(uint32_t index) const { return m_Parts.at(index).indexBuffer; }

private:
    std::string m_Name;
    
    std::vector<Part> m_Parts;  

    // Resource location
    std::string m_ServerResource;
    std::string m_LocalResource;
};