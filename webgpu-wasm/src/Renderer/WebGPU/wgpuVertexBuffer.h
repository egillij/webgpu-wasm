#pragma once

#include <webgpu/webgpu_cpp.h>

#include "wgpuBuffer.h"

//TODO: Bæta við buffer descriptions, s.s. buffer layout (hvaða attributes eru til staðar og hvernig er þeim raðað)
class WGpuVertexBuffer : public WGpuBuffer {
public:
    WGpuVertexBuffer() {};
    WGpuVertexBuffer(WGpuDevice* device, const std::string& label, void* data, size_t size);
    ~WGpuVertexBuffer();
};