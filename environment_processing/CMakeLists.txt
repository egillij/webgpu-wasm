
cmake_minimum_required(VERSION 3.21)
project(environment_processor)
# add_subdirectory(../3pp/glm)
#Set binary directory

SET(SOURCES 
	environment_processor.cpp
)

add_executable(environment_processor ${SOURCES})

# target_link_libraries(model_maker PUBLIC glm)
target_include_directories(environment_processor PUBLIC ../3pp/glm/ ../3pp/ ../utils)