
#include <cstdint>
#include <cstdio>
#include <memory>


// #define GEOM_IO_SAVE
// #define GEOM_IO_LOAD
namespace geom {

struct geom_header {
    char iden[4];
    uint64_t vertexDataOffset;
    uint64_t vertexDataSize;
    uint64_t indexDataOffset;
    uint64_t indexDataSize;
};

class Geom {
public:
    Geom(bool temporaryMemory = false) 
    : m_Vertices(nullptr), m_NoVertices(0), m_Indices(nullptr), 
      m_NoIndices(0), m_TemporaryMemory(temporaryMemory)
    {};

    Geom(const float* vertices, uint64_t numberOfVertices, const uint32_t* indices, uint64_t numberOfIndices)
    : m_Vertices(nullptr), m_NoVertices(0), m_Indices(nullptr), m_NoIndices(0), m_TemporaryMemory(false)
    {
        uint64_t verticesSize = numberOfVertices * sizeof(float);
        m_Vertices = (float*)malloc(verticesSize);
        memcpy(m_Vertices, vertices, verticesSize);
        m_NoVertices = numberOfVertices;

        uint64_t indicesSize = numberOfIndices * sizeof(uint32_t);
        m_Indices = (uint32_t*)malloc(indicesSize);
        memcpy(m_Indices, indices, indicesSize);
        m_NoIndices = numberOfIndices;
    };

    ~Geom()
    {
        if(m_TemporaryMemory) return;

        if(m_Vertices) free(m_Vertices);
        if(m_Indices) free(m_Indices);
    };

    float* vertices() const { return m_Vertices; }
    uint64_t noVertices() const { return m_NoVertices; }
    uint32_t* indices() const { return m_Indices; }
    uint64_t noIndices() const { return m_NoIndices; }

private:
    float* m_Vertices;
    uint64_t m_NoVertices; // Should be number of floats in m_Vertices

    uint32_t* m_Indices;
    uint64_t m_NoIndices; // Should be number of uint_32t in m_Indices

    bool m_TemporaryMemory;

    friend class GeomIO;
};

class GeomIO final {
public:
    GeomIO() {};
    ~GeomIO() {};

#ifdef GEOM_IO_SAVE
    bool save(const char* filename, Geom* geom)
    {
        geom_header header;
        memset(&header, 0, sizeof(header));

        std::FILE* fp = std::fopen(filename, "wb");
        if(!fp){
            printf("Failed to open geometry file %s\n", filename);
            return false;
        }

        header.iden[0] = 'g';
        header.iden[1] = 'e';
        header.iden[2] = 'o';
        header.iden[3] = 'm';

        if(std::fwrite(&header, sizeof(header), 1, fp) != 1){
            printf("Failed to write header to file %s\n", filename);
            std::fclose(fp);
            return false;
        }

        header.vertexDataOffset = std::ftell(fp);
        header.vertexDataSize = geom->m_NoVertices * sizeof(float);
        if(std::fwrite(geom->m_Vertices, sizeof(float), geom->m_NoVertices, fp) != geom->m_NoVertices)
        {
            printf("Failed to write vertex data to file %s\n", filename);
            std::fclose(fp);
            return false;
        }

        header.indexDataOffset = std::ftell(fp);
        header.indexDataSize = geom->m_NoIndices * sizeof(uint32_t);

        if(std::fwrite(geom->m_Indices, sizeof(uint32_t), geom->m_NoIndices, fp) != geom->m_NoIndices)
        {
            printf("Failed to write index data to file %s\n", filename);
            std::fclose(fp);
            return false;
        }

        std::fseek(fp, SEEK_SET, 0);
        
        if(std::fwrite(&header, sizeof(header), 1, fp) != 1){
            printf("Failed to write header to file %s\n", filename);
            std::fclose(fp);
            return false;
        }

        std::fflush(fp);
        std::fclose(fp);

        return true;
    };

    bool save(const char* filename, const float* vertices, uint64_t noVertices, const uint32_t* indices, uint64_t noIndices)
    {
        geom_header header;
        memset(&header, 0, sizeof(geom_header));

        std::FILE* fp = std::fopen(filename, "wb");
        if(!fp){
            printf("Failed to open geometry file %s\n", filename);
            return false;
        }

        header.iden[0] = 'g';
        header.iden[1] = 'e';
        header.iden[2] = 'o';
        header.iden[3] = 'm';

        if(std::fwrite(&header, sizeof(header), 1, fp) != 1){
            printf("Failed to write header to file %s\n", filename);
            std::fclose(fp);
            return false;
        }

        header.vertexDataOffset = std::ftell(fp);
        header.vertexDataSize = noVertices * sizeof(float);
        if(std::fwrite(vertices, sizeof(float), noVertices, fp) != noVertices)
        {
            printf("Failed to write vertex data to file %s\n", filename);
            std::fclose(fp);
            return false;
        }

        header.indexDataOffset = std::ftell(fp);
        header.indexDataSize = noIndices * sizeof(uint32_t);
        if(std::fwrite(indices, sizeof(uint32_t), noIndices, fp) != noIndices)
        {
            printf("Failed to write index data to file %s\n", filename);
            std::fclose(fp);
            return false;
        }

        std::fseek(fp, SEEK_SET, 0);
        
        if(std::fwrite(&header, sizeof(header), 1, fp) != 1){
            printf("Failed to write header to file %s\n", filename);
            std::fclose(fp);
            return false;
        }

        std::fflush(fp);
        std::fclose(fp);

        return true;
    };
#endif

#ifdef GEOM_IO_LOAD
    bool load(const char* filename, Geom* geom)
    {
        if(!geom) {
            printf("No geometry class to write to\n");
            return false;
        }

        std::FILE* fp = std::fopen(filename, "rb");
        if(!fp){
            printf("Failed to open geometry file %s\n", filename);
            return false;
        }

        geom_header header{};
        
        if(std::fread(&header, sizeof(header), 1, fp) != 1){
            printf("Failed to load header from geometry file %s\n", filename);
            std::fclose(fp);
            return false;
        }

        geom->m_Vertices = (float*)malloc(header.vertexDataSize);
        geom->m_NoVertices = header.vertexDataSize / sizeof(float);
        if(std::fread(geom->m_Vertices, 1, header.vertexDataSize, fp) != header.vertexDataSize)
        {
            printf("Failed to load vertex data from geometry file %s\n", filename);
            std::fclose(fp);
            return false;
        }

        geom->m_Indices = (uint32_t*)malloc(header.indexDataSize);
        geom->m_NoIndices = header.indexDataSize / sizeof(uint32_t);
        if(std::fread(geom->m_Indices, 1, header.indexDataSize, fp) != header.indexDataSize)
        {
            printf("Failed to load index data from geometry file %s\n", filename);
            std::fclose(fp);
            return false;
        }

        std::fclose(fp);

        return true;
    };

    bool load(char* data, int size, Geom* geom)
    {
        if(!geom) {
            printf("No geometry class to write to\n");
            return false;
        }


        geom_header header{};
        
        if(size < sizeof(header)){
            printf("Can't to load header from in geometry file. Buffer too small\n");
            return false;
        }

        header = *((geom_header*)data);


        // geom->m_Vertices = (float*)malloc(header.vertexDataSize);
        geom->m_NoVertices = header.vertexDataSize / sizeof(float);
        if(size < sizeof(header) + header.vertexDataSize)
        {
            printf("Failed to load vertex data from in-memory geometry file. Buffer too small\n");
            return false;
        }
        geom->m_Vertices = (float*)(data+header.vertexDataOffset);

        // geom->m_Indices = (uint32_t*)malloc(header.indexDataSize);
        geom->m_NoIndices = header.indexDataSize / sizeof(uint32_t);
        if(size < sizeof(header)+header.vertexDataSize + header.indexDataSize)
        {
            printf("Failed to load index data from in-memory geometry file. Buffer too small\n");
            return false;
        }

        geom->m_Indices = (uint32_t*)(data+header.indexDataOffset);

        return true;
    };
#endif
};

}