
SET(CMAKE_EXECUTABLE_SUFFIX .html)

#Set binary directory

SET(WW_SOURCES 
	src/main.cpp
)

add_executable(wgpu_wasm ${WW_SOURCES})

SET(LINKER_FLAGS
-s USE_WEBGPU=1
-o index.html
)

set_target_properties(wgpu_wasm PROPERTIES LINK_FLAGS "-s USE_WEBGPU=1 -o index.html")
target_link_libraries(wgpu_wasm PUBLIC glm)