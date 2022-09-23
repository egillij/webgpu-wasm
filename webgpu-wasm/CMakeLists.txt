
SET(CMAKE_EXECUTABLE_SUFFIX .html)

#Set binary directory

SET(WW_SOURCES 
	src/main.cpp

	src/Application.h
	src/Application.cpp

	src/Renderer/WebGPU/wgpuBindGroup.h
	src/Renderer/WebGPU/wgpuBindGroup.cpp

	src/Renderer/WebGPU/wgpuBuffer.h
	src/Renderer/WebGPU/wgpuBuffer.cpp

	src/Renderer/WebGPU/wgpuDevice.h
	src/Renderer/WebGPU/wgpuDevice.cpp

	src/Renderer/WebGPU/wgpuIndexBuffer.h
	src/Renderer/WebGPU/wgpuIndexBuffer.cpp

	src/Renderer/WebGPU/wgpuPipeline.h
	src/Renderer/WebGPU/wgpuPipeline.cpp

	src/Renderer/WebGPU/wgpuSampler.h
	src/Renderer/WebGPU/wgpuSampler.cpp

	src/Renderer/WebGPU/wgpuShader.h
	src/Renderer/WebGPU/wgpuShader.cpp

	src/Renderer/WebGPU/wgpuSwapChain.h
	src/Renderer/WebGPU/wgpuSwapChain.cpp

	src/Renderer/WebGPU/wgpuTexture.h
	src/Renderer/WebGPU/wgpuTexture.cpp

	src/Renderer/WebGPU/wgpuUniformBuffer.h
	src/Renderer/WebGPU/wgpuUniformBuffer.cpp

	src/Renderer/WebGPU/wgpuVertexBuffer.h
	src/Renderer/WebGPU/wgpuVertexBuffer.cpp	
)

add_executable(wgpu_wasm ${WW_SOURCES})

SET(LINKER_FLAGS
-s USE_WEBGPU=1
-o index.html
-s ASSERTIONS=2
-s ASYNCIFY
)

set_target_properties(wgpu_wasm PROPERTIES LINK_FLAGS "-s USE_WEBGPU=1 -o index.html -s ASYNCIFY -s ALLOW_MEMORY_GROWTH")
target_link_libraries(wgpu_wasm PUBLIC glm)
target_include_directories(wgpu_wasm PUBLIC ../3pp/)