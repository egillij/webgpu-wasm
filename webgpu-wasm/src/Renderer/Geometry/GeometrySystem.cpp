#include "GeometrySystem.h"

#define GEOM_IO_LOAD
#include "geomIo.h"

GeometrySystem::GeometrySystem(WGpuDevice* device)
: m_Device(device)
{
}

GeometrySystem::~GeometrySystem()
{}

TriangleMesh* GeometrySystem::registerTriangleMesh(uint32_t id, const std::string& name, const std::string& filename)
{
    // Check if a mesh with this id already exists and return it
    const auto it = m_TriangleMeshes.find(id);
    if(it != m_TriangleMeshes.end()){
        return m_TriangleMeshes.at(id).get();
    }

    // Mesh did not exist. Make it and return the results
    geom::GeomIO io = geom::GeomIO();

    geom::Geom geom = geom::Geom();
    bool success = io.load(filename.c_str(), &geom);
    if(success){
        std::shared_ptr<TriangleMesh> mesh = std::make_shared<TriangleMesh>(id, name);

        mesh->update(geom.vertices(), geom.noVertices()*sizeof(float), geom.indices(), geom.noIndices(), m_Device);

        m_TriangleMeshes[id] = mesh;

        return mesh.get();
    }

    return nullptr;
}

TriangleMesh* GeometrySystem::find(uint32_t id)
{
    // Check if a mesh with this id already exists and return it
    const auto it = m_TriangleMeshes.find(id);
    if(it != m_TriangleMeshes.end()){
        return m_TriangleMeshes.at(id).get();
    }

    printf("No mesh exists with id: %u\n", id);
    return nullptr;
}