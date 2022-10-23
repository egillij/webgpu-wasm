#pragma once
// #define MATS_IO_SAVE
// #define MATS_IO_LOAD

#include <cstdint>
#include <cstdio>
#include <string>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace mats {

enum class MaterialType : uint32_t{
    Invalid = 0,
    Phong,
    PBR
};

struct PhongMaterial {
    glm::vec4 ambient;
    glm::vec4 albedo;
    glm::vec4 specular;
    float shininess;

    uint64_t ambientTextureSize;
    char ambientTexture[512];
    uint64_t albedoTextureSize;
    char albedoTexture[512];
    uint64_t specularTextureSize;
    char specularTexture[512];
};

struct PBRMaterial {
    glm::vec3 albedo;
    float metallic;
    float roughness;
    float ao;

    uint64_t albedoTextureSize;
    char albedoTexture[512];
    uint64_t metallicTextureSize;
    char metallicTexture[512];
    uint64_t roughnessTextureSize;
    char roughnessTexture[512];
    uint64_t aoTextureSize;
    char aoTexture[512];
};

// TODO: extend to support more material types
struct MaterialParameters {
    glm::vec3 albedo;
    float metallic;
    float roughness;
    float ao;
    

    uint64_t albedoTextureSize;
    char albedoTexture[512];
    uint64_t metallicTextureSize;
    char metallicTexture[512];
    uint64_t roughnessTextureSize;
    char roughnessTexture[512];
    uint64_t aoTextureSize;
    char aoTexture[512];
};

struct mats_header {
    char iden[4];
    uint32_t type; //MaterialType
};

class MatsIO final {
public:
    MatsIO() {};
    ~MatsIO() {};

#ifdef MATS_IO_SAVE
    bool save(const char* filename, const MaterialParameters& data)
    {
        mats_header header{};
        memset(&header, 0, sizeof(header));

        header.iden[0] = 'm';
        header.iden[1] = 'a';
        header.iden[2] = 't';
        header.iden[3] = 's';

        header.type = (uint32_t)MaterialType::PBR;

        std::FILE* fp = std::fopen(filename, "wb");

        if(!fp) 
        {
            printf("Failed to open material file %s\n", filename);
            return false;
        }

        if(std::fwrite(&header, sizeof(header), 1, fp) != 1)
        {
            printf("Failed to write header to material file %s\n", filename);
            std::fclose(fp);
            return false;
        }

        if(std::fwrite(&data, sizeof(MaterialParameters), 1, fp) != 1)
        {
            printf("Failed to write material data to file %s\n", filename);
            std::fclose(fp);
            return false;
        }

        std::fflush(fp);
        std::fclose(fp);

        return true;
    };
#endif

#ifdef MATS_IO_LOAD
    bool load(const char* filename, MaterialParameters* material)
    {
        if(!material){
            printf("No data to write to\n");
            return false;
        }

        std::FILE* fp = std::fopen(filename, "rb");
        mats_header header{};
        memset(&header, 0, sizeof(header));

        if(std::fread(&header, sizeof(header), 1, fp) != 1)
        {
            printf("Failed to read header from material file %s\n", filename);
            std::fclose(fp);
            return false;
        }

        if(std::fread(material, sizeof(MaterialParameters), 1, fp) != 1)
        {
            printf("Failed to load material data from file %s\n", filename);
            std::fclose(fp);
            return false;
        }
        
        std::fclose(fp);
        return true;
    };

    bool load(const char* data, uint64_t size, MaterialParameters* material) 
    {
        if(size != sizeof(mats_header) + sizeof(MaterialParameters)){
            printf("Invalid size of material file buffer\n");
            return false;
        }

        mats_header* header = (mats_header*)&data[0];
        memcpy(material, data+sizeof(mats_header), sizeof(MaterialParameters));

        return true;
    }
#endif
};

}