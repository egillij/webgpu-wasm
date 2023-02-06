#pragma once

#include <webgpu/webgpu_cpp.h>

class WGpuDevice final {
public:
    WGpuDevice() {};
    WGpuDevice(wgpu::Device device);
    ~WGpuDevice();

    wgpu::Device& getHandle() {return m_Device;}

private:
    wgpu::Device m_Device;
};