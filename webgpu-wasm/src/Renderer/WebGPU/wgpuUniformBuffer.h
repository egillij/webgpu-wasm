// Copyright 2023 Egill Ingi Jacobsen

#pragma once

#include <webgpu/webgpu_cpp.h>

#include "wgpuBuffer.h"

//TODO: vill ég gera einhverskonar uniformbufferlayout til að lýsa hvað er í þessum buffer og það getur svo reiknað stærðina sem þarf?
class WGpuUniformBuffer : public WGpuBuffer {
public:
    WGpuUniformBuffer() {};
    WGpuUniformBuffer(WGpuDevice* device, const std::string& label, uint64_t size);
    ~WGpuUniformBuffer() {};
};