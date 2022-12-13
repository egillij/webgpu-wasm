#include "GeometrySystem.h"

#define GEOM_IO_LOAD
#include "geomIo.h"

#include <emscripten.h>

struct LoadData {
    GeometrySystem* geoSystem;
    uint32_t meshId;
};

void onMeshLoadSuccess(void* userData, void* data, int size)
{
    if(!userData)
    {
        printf("Geometry system lost. Can't update mesh!\n");
        return;
    }

    LoadData* loadData = (LoadData*) userData;
    loadData->geoSystem->updateTriangleMesh(loadData->meshId, data, size);

    delete loadData;
}

void onMeshLoadError(void* userData) 
{
    if(!userData){
        printf("Failed to load mesh. Geometry system lost\n");
    }
    else {
        LoadData* loadData = (LoadData*)userData;
        printf("Failed to load mesh %u\n", loadData->meshId);
        delete loadData;
    }
}

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

    std::shared_ptr<TriangleMesh> mesh = std::make_shared<TriangleMesh>(id, name);

    m_TriangleMeshes[id] = mesh;

    LoadData* userData = new LoadData;
    userData->geoSystem = this;
    userData->meshId = id;

    emscripten_async_wget_data(filename.c_str(), (void*)userData, onMeshLoadSuccess, onMeshLoadError);

    return mesh.get();
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

void GeometrySystem::updateTriangleMesh(uint32_t id, void* data, int size) 
{
    const auto it = m_TriangleMeshes.find(id);
    if(it == m_TriangleMeshes.end()){
        printf("Could not find mesh with id %u. No update done.\n", id);
        return;
    }

    auto mesh = it->second;
    geom::GeomIO io = geom::GeomIO();

    geom::Geom geom = geom::Geom(true);
    bool success = io.load((char*)data, size, &geom);
    if(success){
        mesh->update(geom.vertices(), geom.noVertices()*sizeof(float), geom.indices(), geom.noIndices(), m_Device);
    }
    
}

void GeometrySystem::clear()
{
    m_TriangleMeshes.clear();
}