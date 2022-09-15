
SET(CMAKE_EXECUTABLE_SUFFIX .html)

#Set binary directory

SET(WW_SOURCES 
	src/main.cpp

	src/Renderer/WebGPU/wgpuDevice.h
	src/Renderer/WebGPU/wgpuDevice.cpp

	src/Renderer/WebGPU/wgpuSwapChain.h
	src/Renderer/WebGPU/wgpuSwapChain.cpp

	src/Renderer/WebGPU/wgpuBindGroup.h
	src/Renderer/WebGPU/wgpuBindGroup.cpp

	src/Renderer/WebGPU/wgpuBuffer.h
	src/Renderer/WebGPU/wgpuBuffer.cpp

	src/Renderer/WebGPU/wgpuVertexBuffer.h
	src/Renderer/WebGPU/wgpuVertexBuffer.cpp

	src/Renderer/WebGPU/wgpuIndexBuffer.h
	src/Renderer/WebGPU/wgpuIndexBuffer.cpp

	src/Renderer/WebGPU/wgpuUniformBuffer.h
	src/Renderer/WebGPU/wgpuUniformBuffer.cpp
)

add_executable(wgpu_wasm ${WW_SOURCES})

SET(LINKER_FLAGS
-s USE_WEBGPU=1
-o index.html
-s ASSERTIONS=2
)

set_target_properties(wgpu_wasm PROPERTIES LINK_FLAGS "-s USE_WEBGPU=1 -o index.html")
target_link_libraries(wgpu_wasm PUBLIC glm)