#include "GeometrySystem.h"

#define GEOM_IO_LOAD
#include "geomIo.h"

#include <emscripten.h>

constexpr uint32_t UNIT_CUBE_ID = 1u;

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
    m_UnitCube = std::make_unique<TriangleMesh>(UNIT_CUBE_ID, "Unit Cube");

    // Float3 position, Float3 normal, Float2 uv
static float cubeVertices[6*6*8] = {
    //Bottom
    1.f, -1.f, 1.f,     0.f, -1.f, 0.f,    1.f, 1.f,
    -1.f, -1.f, 1.f,    0.f, -1.f, 0.f,    0.f, 1.f,
    -1.f, -1.f, -1.f,   0.f, -1.f, 0.f,    0.f, 0.f,
    1.f, -1.f, -1.f,    0.f, -1.f, 0.f,    1.f, 0.f,
    1.f, -1.f, 1.f,     0.f, -1.f, 0.f,    1.f, 1.f,
    -1.f, -1.f, -1.f,   0.f, -1.f, 0.f,    0.f, 0.f,
    // Right
    1.f, 1.f, 1.f,      1.f, 0.f, 0.f,    1.f, 1.f,
    1.f, -1.f, 1.f,     1.f, 0.f, 0.f,    0.f, 1.f,
    1.f, -1.f, -1.f,    1.f, 0.f, 0.f,    0.f, 0.f,
    1.f, 1.f, -1.f,     1.f, 0.f, 0.f,    1.f, 0.f,
    1.f, 1.f, 1.f,      1.f, 0.f, 0.f,    1.f, 1.f,
    1.f, -1.f, -1.f,    1.f, 0.f, 0.f,    0.f, 0.f,
    //Top
    -1.f, 1.f, 1.f,     0.f, 1.f, 0.f,    1.f, 1.f,
    1.f, 1.f, 1.f,      0.f, 1.f, 0.f,    0.f, 1.f,
    1.f, 1.f, -1.f,     0.f, 1.f, 0.f,    0.f, 0.f,
    -1.f, 1.f, -1.f,    0.f, 1.f, 0.f,    1.f, 0.f,
    -1.f, 1.f, 1.f,     0.f, 1.f, 0.f,    1.f, 1.f,
    1.f, 1.f, -1.f,     0.f, 1.f, 0.f,    0.f, 0.f,

    -1.f, -1.f, 1.f,    -1.f, 0.f, 0.f,    1.f, 1.f,
    -1.f, 1.f, 1.f,     -1.f, 0.f, 0.f,    0.f, 1.f,
    -1.f, 1.f, -1.f,    -1.f, 0.f, 0.f,    0.f, 0.f,
    -1.f, -1.f, -1.f,   -1.f, 0.f, 0.f,    1.f, 0.f,
    -1.f, -1.f, 1.f,    -1.f, 0.f, 0.f,    1.f, 1.f,
    -1.f, 1.f, -1.f,    -1.f, 0.f, 0.f,    0.f, 0.f,

    1.f, 1.f, 1.f,      0.f, 0.f, 1.f,    1.f, 1.f,
    -1.f, 1.f, 1.f,     0.f, 0.f, 1.f,    0.f, 1.f,
    -1.f, -1.f, 1.f,    0.f, 0.f, 1.f,    0.f, 0.f,
    -1.f, -1.f, 1.f,    0.f, 0.f, 1.f,    0.f, 0.f,
    1.f, -1.f, 1.f,     0.f, 0.f, 1.f,    1.f, 0.f,
    1.f, 1.f, 1.f,      0.f, 0.f, 1.f,    1.f, 1.f,

    1.f, -1.f, -1.f,    0.f, 0.f, -1.f,    1.f, 1.f,
    -1.f, -1.f, -1.f,   0.f, 0.f, -1.f,    0.f, 1.f,
    -1.f, 1.f, -1.f,    0.f, 0.f, -1.f,    0.f, 0.f,
    1.f, 1.f, -1.f,     0.f, 0.f, -1.f,    1.f, 0.f,
    1.f, -1.f, -1.f,    0.f, 0.f, -1.f,    1.f, 1.f,
    -1.f, 1.f, -1.f,    0.f, 0.f, -1.f,    0.f, 0.f,
    };

    static uint64_t numCubeIndices = 6*6;
    static uint32_t cubeIndices[6*6] = {
        0, 1, 2,
        3, 4, 5,

        6, 7, 8,
        9, 10, 11,

        12, 13, 14,
        15, 16, 17,

        18, 19, 20,
        21, 22, 23,

        24, 25, 26,
        27, 28, 29,

        30, 31, 32,
        33, 34, 35
    };

    m_UnitCube->update(cubeVertices, numCubeIndices*8*sizeof(float), cubeIndices, numCubeIndices, m_Device);
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

const TriangleMesh* GeometrySystem::getCube() const
{
    return m_UnitCube.get();
}

void GeometrySystem::clear()
{
    m_TriangleMeshes.clear();
}