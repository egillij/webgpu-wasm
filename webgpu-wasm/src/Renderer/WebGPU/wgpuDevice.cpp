// Copyright 2023 Egill Ingi Jacobsen

#include "wgpuDevice.h"

#include <cstdio>

WGpuDevice::WGpuDevice(wgpu::Device device) 
{
    m_Device = device;

    m_Device.SetUncapturedErrorCallback([](WGPUErrorType errorType, const char* message, void* userdata){
        printf("Error: %d -> %s", errorType, message);
    }, nullptr);
}

WGpuDevice::~WGpuDevice()
{
    if(m_Device){
        m_Device.Destroy();
    }
}