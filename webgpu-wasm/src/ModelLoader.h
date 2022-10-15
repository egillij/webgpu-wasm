#pragma once

#include <string>
#include <cstdint>
#include <vector>

// We assume all models will have interleaved vertices with position, normal, uv
struct ModelData {
    struct PartData {
        std::string name;
        float* vertexData = nullptr;
        uint64_t numberOfVertices = 0;
        uint32_t* indexData = nullptr;
        uint64_t numberOfIndices = 0;
    };
    
    std::vector<PartData> modelData;
};

class ModelLoader {
public:
    static ModelData loadModelFromFile(const char* filename);
};