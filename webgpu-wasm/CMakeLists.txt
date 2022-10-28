
SET(CMAKE_EXECUTABLE_SUFFIX .html)
set (CMAKE_CXX_STANDARD 20)
#Set binary directory

SET(WW_SOURCES 
	src/main.cpp

	src/Application.h
	src/Application.cpp

	src/Renderer/Renderer.h
	src/Renderer/Renderer.cpp

	src/Renderer/Material.h
	src/Renderer/Material.cpp
	src/Renderer/PBRMaterial.cpp

	src/Renderer/MaterialSystem.h
	src/Renderer/MaterialSystem.cpp

	src/Renderer/TextureSystem.h
	src/Renderer/TextureSystem.cpp

	src/Renderer/Geometry/GeometrySystem.h
	src/Renderer/Geometry/GeometrySystem.cpp

	src/Renderer/Geometry/TriangleMesh.h
	src/Renderer/Geometry/TriangleMesh.cpp
	
	src/Renderer/Geometry/Part.h
	src/Renderer/Geometry/Part.cpp

	src/Renderer/Pipelines/RenderPipeline.h
	src/Renderer/Pipelines/RenderPipeline.cpp

	src/Renderer/Pipelines/PBRShaders.h
	src/Renderer/Pipelines/PBRRenderPipeline.h
	src/Renderer/Pipelines/PBRRenderPipeline.cpp

	src/Renderer/Pipelines/PresentShader.h
	src/Renderer/Pipelines/PresentPipeline.h
	src/Renderer/Pipelines/PresentPipeline.cpp

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

	src/Scene/GameObject.h
	src/Scene/GameObject.cpp

	src/Scene/Scene.h
	src/Scene/Scene.cpp

	src/Utils/UniformStructs.h
)

add_executable(wgpu_wasm ${WW_SOURCES})

SET(LINKER_FLAGS
-s USE_WEBGPU=1
-o index.html
-s ASSERTIONS=2
-s ASYNCIFY
)

add_compile_definitions("WAIT_FOR_QUEUE")

configure_file(custom_shell.html custom_shell.html COPYONLY)

set_target_properties(wgpu_wasm PROPERTIES LINK_FLAGS "-sUSE_WEBGPU=1 -o index.html -sALLOW_MEMORY_GROWTH -sNO_DISABLE_EXCEPTION_CATCHING --shell-file custom_shell.html")
target_link_libraries(wgpu_wasm PUBLIC glm)
target_include_directories(wgpu_wasm PUBLIC src/ ../3pp/ ../utils/)