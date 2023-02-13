// Copyright 2023 Egill Ingi Jacobsen

#pragma once

#include <string>
#include <cstdint>
#include <vector>

class MaterialSystem;
class Material;

// We assume all models will have interleaved vertices with position, normal, uv
struct ModelData {
    struct PartData {
        std::string name;
        float* vertexData = nullptr;
        uint64_t numberOfVertices = 0;
        uint32_t* indexData = nullptr;
        uint64_t numberOfIndices = 0;
        Material* material = nullptr;
    };
    
    std::vector<PartData> modelData;
};

class ModelLoader {
public:
    static ModelData loadModelFromFile(const char* filename, MaterialSystem* materialSystem);
};